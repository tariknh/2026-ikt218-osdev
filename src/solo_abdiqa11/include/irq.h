#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

void irq_install(void);

extern volatile uint32_t timer_ticks;

#endif
