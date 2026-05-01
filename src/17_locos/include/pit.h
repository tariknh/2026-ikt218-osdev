#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>

void init_pit(void);
void pit_on_irq0(void);
uint32_t pit_get_ticks(void);
void sleep_busy(uint32_t milliseconds);
void sleep_interrupt(uint32_t milliseconds);

#endif