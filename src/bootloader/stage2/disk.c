#include "disk.h"

#include "stdio.h"
#include "x86.h"

bool DISK_Initialize(DISK *disk, uint8_t driveNumber) {
  printf("[ DISK ]: Initializing disk drive %u\n", driveNumber);

  uint8_t driveType;
  uint16_t cylinders, sectors, heads;

  if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors,
                               &heads))
    return false;

  disk->id = driveNumber;
  disk->cylinders = cylinders;
  disk->sectors = sectors;
  disk->heads = heads;

  printf("[ DISK ]: Drive %u ready! Cylinders: %lu, Sectors: %lu, Heads: %lu\n",
         driveNumber, cylinders, sectors, heads);

  return true;
}

void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut,
                  uint16_t *sectorOut, uint16_t *headOut) {
  // sector (LBA % sectors per track + 1)
  *sectorOut = lba % disk->sectors + 1;

  // cylinder (LBA / sectors per track) / heads
  *cylinderOut = (lba / disk->sectors) / disk->heads;

  // head (LBA / sectors per track) % heads
  *headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK *disk, uint32_t lba, uint8_t sectors,
                      void *lowerDataOut) {
  printf("[ DISK ]: Reading sectors of drive %u, LBA: %lu, Sectors: %lu\n",
         disk->id, lba, sectors);
  uint16_t cylinder, sector, head;

  DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

  for (int i = 0; i < 3; i++) {
    if (x86_Disk_Read(disk->id, cylinder, sector, head, sectors, lowerDataOut))
      return true;

    x86_Disk_Reset(disk->id);
  }

  return false;
}