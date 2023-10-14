#include <stdint.h>

#include "memory.h"
#include "stdio.h"

extern uint8_t __bss_start;
extern uint8_t __end;

static uint8_t *g_ScreenBuffer = (uint8_t *)0xB8000;

void __attribute__((section(".entry"))) start() {
  memset(&__bss_start, 0, (&__end) - (&__bss_start));

  g_ScreenBuffer[0] = '#';
  g_ScreenBuffer[1] = 0x0A;

  clrscr();

  printf("Hello, World from Kernel!\n");

end:
  for (;;)
    ;
}
