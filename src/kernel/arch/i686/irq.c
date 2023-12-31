#include "irq.h"

#include "i8259.h"
#include "io.h"
#include "pic.h"
#include "stdio.h"

#include <stddef.h>

#define PIC_REMAP_OFFSET 0x20

IRQHandler g_IRQHandlers[16];
static const PICDriver *g_Driver = NULL;

void i686_IRQ_Handler(Registers *regs) {
  int irq = regs->interrupt - PIC_REMAP_OFFSET;

  if (g_IRQHandlers[irq] != NULL) {
    // Handle IRQ
    g_IRQHandlers[irq](regs);
  } else {
    printf("Unhandled IRQ %d...\n", irq);
  }
  // Send EOI
  g_Driver->SendEndOfInterrupt(irq);
}

void i686_IRQ_Initialize() {
  const PICDriver *drivers[] = {
      i8259_GetDriver(),
  };

  for (int i = 0; i < sizeof(drivers) / sizeof(PICDriver *); i++) {
    if (drivers[i]->Probe()) {
      g_Driver = drivers[i];
      break;
    }
  }

  if (g_Driver == NULL) {
    printf("[ IRQ  ]: No PIC found!\n");
    return;
  }

  printf("[ IRQ  ]: Found %s PIC Driver.\n", g_Driver->Name);
  g_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

  // Register ISR handlers for each of the 16 irq lines
  for (int i = 0; i < 16; i++)
    i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQ_Handler);

  // Enable interrupts
  i686_EnableInterrupts();
}

void i686_IRQ_RegisterHandler(int irq, IRQHandler handler) {
  g_IRQHandlers[irq] = handler;
  g_Driver->Unmask(irq);
}
