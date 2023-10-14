#include <stdint.h>

#include "disk.h"
#include "stdio.h"

void *g_Data = (void *)0x20000;

void __attribute__((cdecl)) start(uint16_t bootDrive) {
  clrscr();

  printf("[STAGE2]: Enter 32-bit protected mode!\n");

  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("[STAGE2]: Disk init error!\n");
    goto end;
  }

  DISK_ReadSectors(&disk, 0, 1, g_Data);
  // print_buffer("First sector:", g_Data, 512);

end:
  printf("[STAGE2]: Halting processor.");
  for (;;)
    ;
}
