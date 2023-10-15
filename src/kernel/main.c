#include <stdint.h>

#include "memory.h"
#include "stdio.h"

extern uint8_t __bss_start;
extern uint8_t __end;

void __attribute__((section(".entry"))) start() {
  memset(&__bss_start, 0, (&__end) - (&__bss_start));

  clrscr();

  printf("[KERNEL]: Enter in kernel mode!\n");
  printf("Test %d of %s\n", 12345, "abcde");

  for (;;)
    ;
}
