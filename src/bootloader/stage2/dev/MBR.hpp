#pragma once

#include <stdint.h>

struct MBREntry {
  uint8_t Attributes;
  uint8_t CHSStart[3];
  uint8_t PartitionType;
  uint8_t CHSEnd[3];
  uint32_t LBAStart;
  uint32_t Size;
} __attribute__((packed));
