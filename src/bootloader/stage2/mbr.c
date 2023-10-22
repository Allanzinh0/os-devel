#include "mbr.h"

#include "disk.h"
#include "memory.h"
#include "stdio.h"

typedef struct {
  uint8_t Attributes;
  uint8_t CHSStart[3];
  uint8_t PartitionType;
  uint8_t CHSEnd[3];
  uint32_t LBAStart;
  uint32_t Size;
} __attribute__((packed)) MBR_Entry;

void MBR_DetectPartition(Partition *part, DISK *disk, void *partition) {
  part->Disk = disk;

  if (disk->id < 0x80) {
    part->Offset = 0;
    part->Size = (uint32_t)(disk->cylinders) * (uint32_t)(disk->heads) *
                 (uint32_t)(disk->sectors);
    return;
  }

  MBR_Entry *entry = (MBR_Entry *)segoffset_to_linear(partition);
  printf("[ MBR  ]: Read entry: Attr:%x, PartitionType:%x, LBAStart:%x, "
         "Size:%x\r\n",
         entry->Attributes, entry->PartitionType, entry->LBAStart, entry->Size);
  part->Offset = entry->LBAStart;
  part->Size = entry->Size;
}

bool Partition_ReadSectors(Partition *part, uint32_t lba, uint8_t sectors,
                           void *lowerDataOut) {
  return DISK_ReadSectors(part->Disk, lba + part->Offset, sectors,
                          lowerDataOut);
}
