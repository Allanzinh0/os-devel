#include <stdint.h>

#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "stdio.h"
#include "vbe.h"

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
  // kernelStart();

  VBE_InfoBlock *info = (VBE_InfoBlock *)MEMORY_VESA_INFO;
  VBE_ModeInfo *modeInfo = (VBE_ModeInfo *)MEMORY_MODE_INFO;

  const int desiredWidth = 1280;
  const int desiredHeight = 720;
  const int desiredBPP = 32;
  uint16_t pickedMode = 0xFFFF;

  if (VBE_GetControllerInfo(info)) {
    printf("[STAGE2]: Found VBE extensions.\n");

    uint16_t *mode = (uint16_t *)(info->VideoModePtr);

    for (int i = 0; mode[i] != 0xFFFF; i++) {
      if (VBE_GetModeInfo(mode[i], modeInfo)) {
        bool hasFB = (modeInfo->Attributes & 0x90) == 0x90;
        if (hasFB && modeInfo->Width == desiredWidth &&
            modeInfo->Height == desiredHeight && modeInfo->BPP == desiredBPP) {
          pickedMode = mode[i];
          break;
        }
      }
    }
    if (pickedMode != 0xFFFF && VBE_SetMode(pickedMode)) {
      uint32_t *fb = (uint32_t *)(modeInfo->Framebuffer);

      for (int y = 0; y < modeInfo->Height; y++) {
        for (int x = 0; x < modeInfo->Width; x++) {
          fb[y * modeInfo->Pitch / 4 + x] = 0xff00ff00;
        }
      }
    }
  } else {
    printf("[STAGE2]: No VBE extensions.\n");
  }
end:
  printf("[STAGE2]: Halt processor");
  for (;;)
    ;
}
