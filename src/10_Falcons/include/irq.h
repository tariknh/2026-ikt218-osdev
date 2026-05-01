#ifndef IRQ_H
#define IRQ_H

#include <libc/stdint.h>

void irq_install(void);
void irq_handler(uint32_t irq_number);

#endif