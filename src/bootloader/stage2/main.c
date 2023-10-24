#include <stdint.h>

#include "disk.h"
#include "elf.h"
#include "fat.h"
#include "mbr.h"
#include "memdefs.h"
#include "memory.h"
#include "stdio.h"
#include "stdlib.h"

#include <stddef.h>

typedef void (*KernelStart)();

void stage2_start(uint16_t bootDrive, void *partition) {
  clrscr();

  printf("[STAGE2]: Enter 32-bit protected mode!\n");

  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("[STAGE2]: Disk init error!\n");
    return;
  }

  Partition part;
  MBR_DetectPartition(&part, &disk, partition);

  if (!FAT_Initialize(&part)) {
    printf("[STAGE2]: FAT init error!\n");
    return;
  }

  // Loading kernel
  KernelStart kernelEntry = NULL;
  printf("[STAGE2]: Loading kernel into memory\n");
  if (!ELF_Open(&part, "/boot/kernel.elf", (void **)&kernelEntry)) {
    printf("[STAGE2]: Couldn't load kernel!\n");
    return;
  }

  printf("[STAGE2]: Loaded kernel into memory\n");

  // Execute Kernel
  printf("[STAGE2]: Executing kernel from memory: 0x%lx\n", kernelEntry);
  kernelEntry();
}

void __attribute__((cdecl)) start(uint16_t bootDrive, void *partition) {
  stage2_start(bootDrive, partition);

  printf("[STAGE2]: Halt processor\n");
  for (;;)
    ;
}
