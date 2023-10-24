#pragma once

// 0x00000000 - 0x000003FF - Interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area
#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

// 0x00020000 - FAT driver
#define MEMORY_FAT_ADDR ((void *)0x20000)
#define MEMORY_FAT_SIZE 0x00010000

// 0x00030000 - Load Elf Data
#define MEMORY_ELF_ADDR ((void *)0x30000)
#define MEMORY_ELF_SIZE 0x00010000

// 0x00040000 - Load Kernel
#define MEMORY_LOAD_KERNEL ((void *)0x40000)
#define MEMORY_LOAD_SIZE 0x00010000

// 0x00030000 - 0x00080000 - free

// 0x00080000 - 0x0009FFFF - Extended BIOS data area
// 0x000A0000 - 0x000C7FFF - Video
// 0x000C8000 - 0x000FFFFF - BIOS

#define MEMORY_KERNEL_ADDR ((void *)0x100000)
