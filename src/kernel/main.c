#include <stdint.h>

#include "hal/hal.h"
#include "memory.h"
#include "stdio.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start() {
  memset(&__bss_start, 0, (&__end) - (&__bss_start));

  HAL_Initialize();

  clrscr();

  printf("[KERNEL]: Enter in kernel mode!\n");

  for (;;)
    ;
}