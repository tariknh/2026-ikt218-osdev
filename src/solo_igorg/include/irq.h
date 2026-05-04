#ifndef IRQ_H
#define IRQ_H

#include <libc/stdint.h>

/*
 * Initializes IRQ support, remaps PIC and registers IRQ0-IRQ15 in IDT.
 */
void irq_initialize(void);

/*
 * Common C-level IRQ handler called from assembly stubs.
 */
void irq_handler(uint32_t irq_number);

#endif
