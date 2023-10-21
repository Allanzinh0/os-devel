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

uint32_t MBR_DetectPartition(DISK *disk, void *partition) {
  if (disk->id < 0x80)
    return 0;

  MBR_Entry *entry = (MBR_Entry *)segoffset_to_linear(partition);
  printf("[ MBR  ]: Read entry: Attr:%x, PartitionType:%x, LBAStart:%x, "
         "Size:%x\r\n",
         entry->Attributes, entry->PartitionType, entry->LBAStart, entry->Size);

  return entry->LBAStart;
}
