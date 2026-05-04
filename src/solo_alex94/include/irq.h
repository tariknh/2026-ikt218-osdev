#ifndef IRQ_H
#define IRQ_H

#include <libc/stdint.h>
#include "isr.h"

typedef void (*irq_callback_t)(struct registers* regs);

void irq_install(void);
void irq_handler(struct registers* regs);
void register_irq_handler(uint8_t irq_line, irq_callback_t handler);

#endif
