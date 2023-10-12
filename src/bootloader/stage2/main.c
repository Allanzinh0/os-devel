#include "disk.h"
#include "fat.h"
#include "stdint.h"
#include "stdio.h"

void _cdecl cstart_(uint16_t bootDrive) {
  printf("MAIN: Stage 2 initialized\r\n");

  DISK disk;
  if (!DISK_Initialize(&disk, bootDrive)) {
    printf("MAIN: Disk init error\r\n");
    goto end;
  }

  if (!FAT_Initialize(&disk)) {
    printf("MAIN: FAT init error\r\n");
    goto end;
  }

  FAT_File far *fd = FAT_Open(&disk, "/");
  FAT_DirectoryEntry entry;

  int i = 0;
  while (FAT_ReadEntry(&disk, fd, &entry) && i++ < 6) {
    for (int i = 0; i < 11; i++)
      putc(entry.Name[i]);
    printf("\r\n");
  }

  FAT_Close(fd);

  // read test.txt
  char buffer[100];
  uint32_t read;
  fd = FAT_Open(&disk, "test.txt");
  while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
    for (uint32_t i = 0; i < read; i++) {
      putc(buffer[i]);
    }
  }
  FAT_Close(fd);
end:
  for (;;)
    ;
}
