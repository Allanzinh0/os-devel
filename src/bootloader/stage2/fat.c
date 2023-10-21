#include "fat.h"

#include <stddef.h>

#include "ctype.h"
#include "disk.h"
#include "memdefs.h"
#include "memory.h"
#include "minmax.h"
#include "stdio.h"
#include "string.h"
#include "utility.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1

typedef struct {
  uint8_t BootJumpInstruction[3];
  uint8_t OemIdentifier[8];
  uint16_t BytesPerSector;
  uint8_t SectorsPerCluster;
  uint16_t ReservedSectors;
  uint8_t FatCount;
  uint16_t DirEntriesCount;
  uint16_t TotalSectors;
  uint8_t MediaDescriptorType;
  uint16_t SectorsPerFat;
  uint16_t SectorsPerTrack;
  uint16_t Heads;
  uint32_t HiddenSectors;
  uint32_t LargeSectorCount;

  // extended boot record
  uint8_t DriveNumber;
  uint8_t _Reserved;
  uint8_t Signature;
  uint32_t VolumeId;
  uint8_t VolumeLabel[11];
  uint8_t SystemId[8];
} __attribute__((packed)) FAT_BootSector;

typedef struct {
  FAT_File Public;
  bool Opened;
  uint32_t FirstCluster;
  uint32_t CurrentCluster;
  uint32_t CurrentSectorInCluster;
  uint8_t Buffer[SECTOR_SIZE];
} FAT_FileData;

typedef struct {
  union {
    FAT_BootSector BootSector;
    uint8_t BootSectorBytes[SECTOR_SIZE];
  } BS;

  FAT_FileData RootDirectory;

  FAT_FileData OpenedFiles[MAX_FILE_HANDLES];

} FAT_Data;

static FAT_Data *g_Data;
static uint8_t *g_Fat;
static uint32_t g_DataSectionLba;
static uint8_t g_FatType;

bool FAT_ReadBootSector(DISK *disk) {
  return DISK_ReadSectors(disk, 0, 1, &g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(DISK *disk) {
  return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors,
                          g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

void FAT_Detect(DISK *disk) {}

bool FAT_Initialize(DISK *disk) {
  g_Data = (FAT_Data *)MEMORY_FAT_ADDR;

  printf("[ FAT  ]: Reading boot sector of drive %u\n", disk->id);
  if (!FAT_ReadBootSector(disk)) {
    printf("[ FAT  ]: Read boot sector failed\n");
    return false;
  }

  g_Fat = (uint8_t *)g_Data + sizeof(FAT_Data);
  uint32_t fatSize = g_Data->BS.BootSector.SectorsPerFat *
                     g_Data->BS.BootSector.BytesPerSector;

  if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
    printf("[ FAT  ]: Not enougth memory to read FAT! Required %lu, only have "
           "%u\n",
           sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
    return false;
  }

  printf("[ FAT  ]: Reading FAT table of drive %u\n", disk->id);
  if (!FAT_ReadFat(disk)) {
    printf("[ FAT  ]: Read FAT failed\n");
    return false;
  }

  // Open root directory file
  uint32_t rootDirLba =
      g_Data->BS.BootSector.ReservedSectors +
      g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;

  uint32_t rootDirSize =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;

  g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
  g_Data->RootDirectory.Public.IsDirectory = true;
  g_Data->RootDirectory.Public.Position = 0;
  g_Data->RootDirectory.Public.Size =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
  g_Data->RootDirectory.Opened = true;
  g_Data->RootDirectory.FirstCluster = rootDirLba;
  g_Data->RootDirectory.CurrentCluster = rootDirLba;
  g_Data->RootDirectory.CurrentSectorInCluster = 0;

  printf("[ FAT  ]: Reading root directory of drive %u\n", disk->id);
  if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
    printf("[ FAT  ]: Read root directory failed\r\n");
    return false;
  }

  // Calculate data section
  uint32_t rootDirSectors =
      (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /
      g_Data->BS.BootSector.BytesPerSector;
  g_DataSectionLba = rootDirLba + rootDirSectors;

  for (int i = 0; i < MAX_FILE_HANDLES; i++) {
    g_Data->OpenedFiles[i].Opened = false;
  }

  printf("[ FAT  ]: FAT Drive %u initialized\n", disk->id);
  return true;
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
  return g_DataSectionLba +
         (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

FAT_File *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry) {
  int handle = -1;
  for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
    if (!g_Data->OpenedFiles[i].Opened)
      handle = i;
  }

  if (handle < 0) {
    printf("[ FAT  ]: Out of file handles\n");
    return NULL;
  }

  FAT_FileData *fd = &g_Data->OpenedFiles[handle];
  fd->Public.Handle = handle;
  fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
  fd->Public.Position = 0;
  fd->Public.Size = entry->Size;
  fd->FirstCluster =
      entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
  fd->CurrentCluster = fd->FirstCluster;
  fd->CurrentSectorInCluster = 0;

  if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1,
                        fd->Buffer)) {
    printf("[ FAT  ]: Read error\n");
    return NULL;
  }

  fd->Opened = true;
  return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster) {
  uint32_t fatIndex = currentCluster * 3 / 2;
  uint32_t nextCluster = 0;

  if (currentCluster % 2 == 0)
    nextCluster = (*(uint16_t *)(g_Fat + fatIndex)) & 0x0FFF;
  else
    nextCluster = (*(uint16_t *)(g_Fat + fatIndex)) >> 4;

  return nextCluster;
}

uint32_t FAT_Read(DISK *disk, FAT_File *file, uint32_t byteCount,
                  void *dataOut) {
  // get file data
  FAT_FileData *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
                         ? &g_Data->RootDirectory
                         : &g_Data->OpenedFiles[file->Handle];
  uint8_t *u8DataOut = (uint8_t *)dataOut;

  if (!fd->Public.IsDirectory ||
      (fd->Public.IsDirectory && fd->Public.Size != 0))
    byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

  while (byteCount > 0) {
    uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
    uint32_t take = min(byteCount, leftInBuffer);

    memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
    u8DataOut += take;
    fd->Public.Position += take;
    byteCount -= take;

    if (leftInBuffer == take) {
      if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
        ++fd->CurrentCluster;
        if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
          printf("[ FAT  ]: Read error!\n");
          break;
        }
      } else {
        if (++fd->CurrentSectorInCluster >=
            g_Data->BS.BootSector.SectorsPerCluster) {
          fd->CurrentSectorInCluster = 0;
          // printf("CurrentCluster: %lu, NextCluster: %lu\n",
          // fd->CurrentCluster, FAT_NextCluster(fd->CurrentCluster));
          fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
        }

        if (fd->CurrentCluster >= 0xFF8) {
          fd->Public.Size = fd->Public.Position;
          break;
        }

        if (!DISK_ReadSectors(disk,
                              FAT_ClusterToLba(fd->CurrentCluster) +
                                  fd->CurrentSectorInCluster,
                              1, fd->Buffer)) {
          printf("[ FAT  ]: Read error!\n");
          break;
        }
      }
    }
  }

  return u8DataOut - (uint8_t *)dataOut;
}

bool FAT_ReadEntry(DISK *disk, FAT_File *file, FAT_DirectoryEntry *dirEntry) {
  return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) ==
         sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File *file) {
  if (file->Handle == ROOT_DIRECTORY_HANDLE) {
    file->Position = 0;
    g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
  } else {
    g_Data->OpenedFiles[file->Handle].Opened = false;
  }
}

bool FAT_FindFile(DISK *disk, FAT_File *file, const char *name,
                  FAT_DirectoryEntry *entryOut) {
  char fatName[12];
  FAT_DirectoryEntry entry;

  memset(fatName, ' ', sizeof(fatName));
  const char *ext = strchr(name, '.');

  if (ext == NULL)
    ext = name + 11;

  for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
    fatName[i] = toupper(name[i]);

  if (ext != name + 11) {
    for (int i = 0; i < 3 && ext[i + 1]; i++)
      fatName[i + 8] = toupper(ext[i + 1]);
  }

  while (FAT_ReadEntry(disk, file, &entry)) {
    if (memcmp(fatName, entry.Name, 11) == 0) {
      *entryOut = entry;
      return true;
    }
  }

  return false;
}

FAT_File *FAT_Open(DISK *disk, const char *path) {
  char name[MAX_PATH_SIZE];

  if (path[0] == '/')
    path++;

  FAT_File *current = &g_Data->RootDirectory.Public;

  while (*path) {
    bool isLast = false;
    const char *delim = strchr(path, '/');
    if (delim != NULL) {
      memcpy(name, path, delim - path);
      name[delim - path + 1] = '\0';
      path = delim + 1;
    } else {
      uint32_t len = strlen(path);
      memcpy(name, path, len);
      name[len + 1] = '\0';
      path += len;
      isLast = true;
    }

    FAT_DirectoryEntry entry;
    if (FAT_FindFile(disk, current, name, &entry)) {
      FAT_Close(current);

      if (!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0) {
        printf("[ FAT  ]: %s not a directory\n", name);
        return NULL;
      }

      current = FAT_OpenEntry(disk, &entry);
    } else {
      printf("[ FAT  ]: %s not found\n", name);
      return NULL;
    }
  }

  return current;
}
