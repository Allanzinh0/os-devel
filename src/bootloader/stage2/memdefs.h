#pragma once

// 0x00000000 - 0x000003FF - Interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area
#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

// 0x00020000 - FAT driver
#define MEMORY_FAT_ADDR ((void *)0x20000)
#define MEMORY_FAT_SIZE 0x00010000

// 0x00030000 - Load Kernel
#define MEMORY_LOAD_KERNEL ((void *)0x30000)
#define MEMORY_LOAD_SIZE 0x00010000

#define MEMORY_VESA_INFO ((void *)0x40000)
#define MEMORY_MODE_INFO ((void *)0x50000)

// 0x00100000 - Kernel Address
#define MEMORY_KERNEL_ADDR ((void *)0x100000)
