#include <stdint.h>

#include "arch/i686/irq.h"
#include "hal/hal.h"
#include "memory.h"
#include "stdio.h"

void timer(Registers *regs) { printf("."); }

void start() {
  clrscr();
  printf("[KERNEL]: Enter in kernel mode!\n");

  HAL_Initialize();

  i686_IRQ_RegisterHandler(0, timer);

  for (;;)
    ;
}
