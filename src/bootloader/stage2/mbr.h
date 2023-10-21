#pragma once

#include "disk.h"

#include <stdint.h>

uint32_t MBR_DetectPartition(DISK *disk, void *partition);
