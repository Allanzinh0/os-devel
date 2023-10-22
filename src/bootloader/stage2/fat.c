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
#define FAT_CACHE_SIZE 5

typedef struct {
  uint8_t DriveNumber;
  uint8_t _Reserved;
  uint8_t Signature;
  uint32_t VolumeId;
  uint8_t VolumeLabel[11];
  uint8_t SystemId[8];
} __attribute__((packed)) FAT_ExtendedBootRecord;

typedef struct {
  uint32_t SectorsPerFat;
  uint16_t Flags;
  uint16_t FATVersionNumber;
  uint32_t RootDirCluster;
  uint16_t FSInfoSector;
  uint16_t BackupBootSector;
  uint8_t _Reserved[12];
  FAT_ExtendedBootRecord EBR;
} __attribute__((packed)) FAT32_ExtendedBootRecord;

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
  union {
    FAT_ExtendedBootRecord EBR1216;
    FAT32_ExtendedBootRecord EBR32;
  };
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

  uint8_t FatCache[FAT_CACHE_SIZE * SECTOR_SIZE];
  uint32_t FatCachePosition;
} FAT_Data;

static FAT_Data *g_Data;
static uint32_t g_DataSectionLba;
static uint8_t g_FatType;
static uint32_t g_TotalSectors;
static uint32_t g_SectorsPerFat;

bool FAT_ReadBootSector(Partition *disk) {
  return Partition_ReadSectors(disk, 0, 1, &g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(Partition *disk, size_t lbaIndex) {
  g_Data->FatCachePosition = lbaIndex;
  return Partition_ReadSectors(disk,
                               g_Data->BS.BootSector.ReservedSectors + lbaIndex,
                               FAT_CACHE_SIZE, g_Data->FatCache);
}

void FAT_Detect(Partition *disk) {
  uint32_t dataClusters = (g_TotalSectors - g_DataSectionLba) /
                          g_Data->BS.BootSector.SectorsPerCluster;

  if (dataClusters < 0xFF5) {
    g_FatType = 12;
    return;
  }
  if (g_Data->BS.BootSector.SectorsPerFat != 0) {
    g_FatType = 16;
    return;
  }
  g_FatType = 32;
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
  return g_DataSectionLba +
         (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

bool FAT_Initialize(Partition *disk) {
  g_Data = (FAT_Data *)MEMORY_FAT_ADDR;

  printf("[ FAT  ]: Reading boot sector of drive %u\n", disk->Disk->id);
  if (!FAT_ReadBootSector(disk)) {
    printf("[ FAT  ]: Read boot sector failed\n");
    return false;
  }

  // Read FAT
  g_Data->FatCachePosition = 0xFFFFFFFF;

  g_TotalSectors = g_Data->BS.BootSector.TotalSectors;
  if (g_TotalSectors == 0) {
    g_FatType = 32;
    g_TotalSectors = g_Data->BS.BootSector.LargeSectorCount;
  }

  g_SectorsPerFat = g_Data->BS.BootSector.SectorsPerFat;
  if (g_SectorsPerFat == 0) {
    g_FatType = 32;
    g_SectorsPerFat = g_Data->BS.BootSector.EBR32.SectorsPerFat;
  }

  // Open root directory file
  uint32_t rootDirLba;
  uint32_t rootDirSize;
  if (g_FatType == 32) {
    g_DataSectionLba = g_Data->BS.BootSector.ReservedSectors +
                       g_SectorsPerFat * g_Data->BS.BootSector.FatCount;

    rootDirLba = FAT_ClusterToLba(g_Data->BS.BootSector.EBR32.RootDirCluster);
    rootDirSize = 0;
  } else {
    rootDirLba = g_Data->BS.BootSector.ReservedSectors +
                 g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
    rootDirSize =
        sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
    uint32_t rootDirSectors =
        (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /
        g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;
  }

  g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
  g_Data->RootDirectory.Public.IsDirectory = true;
  g_Data->RootDirectory.Public.Position = 0;
  g_Data->RootDirectory.Public.Size =
      sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntriesCount;
  g_Data->RootDirectory.Opened = true;
  g_Data->RootDirectory.FirstCluster = rootDirLba;
  g_Data->RootDirectory.CurrentCluster = rootDirLba;
  g_Data->RootDirectory.CurrentSectorInCluster = 0;

  printf("[ FAT  ]: Reading root directory of drive %u\n", disk->Disk->id);
  if (!Partition_ReadSectors(disk, rootDirLba, 1,
                             g_Data->RootDirectory.Buffer)) {
    printf("[ FAT  ]: Read root directory failed\r\n");
    return false;
  }

  for (int i = 0; i < MAX_FILE_HANDLES; i++) {
    g_Data->OpenedFiles[i].Opened = false;
  }

  FAT_Detect(disk);
  printf("[ FAT  ]: Detected FAT%u on disk!\n", g_FatType);

  printf("[ FAT  ]: FAT Drive %u initialized\n", disk->Disk->id);
  return true;
}

FAT_File *FAT_OpenEntry(Partition *disk, FAT_DirectoryEntry *entry) {
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

  if (!Partition_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1,
                             fd->Buffer)) {
    printf("[ FAT  ]: Read error\n");
    return NULL;
  }

  fd->Opened = true;
  return &fd->Public;
}

uint32_t FAT_NextCluster(Partition *disk, uint32_t currentCluster) {
  // Determine the byte offset
  uint32_t fatIndex;
  if (g_FatType == 12) {
    fatIndex = currentCluster * 3 / 2;
  } else if (g_FatType == 16) {
    fatIndex = currentCluster * 2;
  } else if (g_FatType == 32) {
    fatIndex = currentCluster * 4;
  }

  // Cache has the right number
  uint32_t fatIndexSector = fatIndex / SECTOR_SIZE;
  if (fatIndexSector < g_Data->FatCachePosition ||
      fatIndexSector >= g_Data->FatCachePosition + FAT_CACHE_SIZE) {
    FAT_ReadFat(disk, fatIndexSector);
  }

  fatIndex -= (g_Data->FatCachePosition * SECTOR_SIZE);

  uint32_t nextCluster;

  if (g_FatType == 12) {
    if (currentCluster % 2 == 0)
      nextCluster = (*(uint16_t *)(g_Data->FatCache + fatIndex)) & 0x0FFF;
    else
      nextCluster = (*(uint16_t *)(g_Data->FatCache + fatIndex)) >> 4;

    if (nextCluster >= 0xFF8)
      nextCluster |= 0xFFFFF000;

    return nextCluster;
  }
  if (g_FatType == 16) {
    nextCluster = (*(uint16_t *)(g_Data->FatCache + fatIndex));

    if (nextCluster >= 0xFFF8)
      nextCluster |= 0xFFFF0000;

    return nextCluster;
  }

  // if (g_FatType == 32)
  return (*(uint32_t *)(g_Data->FatCache + fatIndex));
}

uint32_t FAT_Read(Partition *disk, FAT_File *file, uint32_t byteCount,
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
        if (!Partition_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
          printf("[ FAT  ]: Read error!\n");
          break;
        }
      } else {
        if (++fd->CurrentSectorInCluster >=
            g_Data->BS.BootSector.SectorsPerCluster) {
          fd->CurrentSectorInCluster = 0;
          // printf("CurrentCluster: %lu, NextCluster: %lu\n",
          // fd->CurrentCluster, FAT_NextCluster(fd->CurrentCluster));
          fd->CurrentCluster = FAT_NextCluster(disk, fd->CurrentCluster);
        }

        if (fd->CurrentCluster >= 0xFFFFFFF8) {
          fd->Public.Size = fd->Public.Position;
          break;
        }

        if (!Partition_ReadSectors(disk,
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

bool FAT_ReadEntry(Partition *disk, FAT_File *file,
                   FAT_DirectoryEntry *dirEntry) {
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

bool FAT_FindFile(Partition *disk, FAT_File *file, const char *name,
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

FAT_File *FAT_Open(Partition *disk, const char *path) {
  char name[MAX_PATH_SIZE];

  if (path[0] == '/')
    path++;

  FAT_File *current = &g_Data->RootDirectory.Public;

  while (*path) {
    bool isLast = false;
    const char *delim = strchr(path, '/');
    if (delim != NULL) {
      memcpy(name, path, delim - path);
      name[delim - path] = '\0';
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
