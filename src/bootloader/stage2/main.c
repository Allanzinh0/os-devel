#include <stdint.h>

#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "stdio.h"

uint8_t *KernelLoadBuffer = (uint8_t *)MEMORY_LOAD_KERNEL;
uint8_t *Kernel = (uint8_t *)MEMORY_KERNEL_ADDR;

typedef void (*KernelStart)();

void __attribute__((cdecl)) start(uint16_t bootDrive) {
  clrscr();

  printf("[STAGE2]: Enter 32-bit protected mode!\n");

  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("[STAGE2]: Disk init error!\n");
    goto end;
  }

  if (!FAT_Initialize(&disk)) {
    printf("[STAGE2]: FAT init error!\n");
    goto end;
  }

  // Loading kernel
  printf("[STAGE2]: Loading kernel into memory\n");
  FAT_File *fd = FAT_Open(&disk, "/kernel.bin");
  uint32_t read;
  uint8_t *kernelBuffer = Kernel;
  while ((read = FAT_Read(&disk, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer))) {
    memcpy(kernelBuffer, KernelLoadBuffer, read);
    kernelBuffer += read;
    printf("[STAGE2]: Loading %lu bytes into memory.\n", read);
  }

  FAT_Close(fd);
  printf("[STAGE2]: Loaded kernel into memory\n");

  // Execute Kernel
  printf("[STAGE2]: Executing kernel from memory: 0x%lx\n", Kernel);
  KernelStart kernelStart = (KernelStart)Kernel;
  kernelStart();
end:
  printf("[STAGE2]: Halt processor");
  for (;;)
    ;
}
