#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>

// Program the PIT timer
void init_pit(void);
// Tick hook used by IRQ0
void pit_on_irq0(void);
// Read the current tick count
uint32_t pit_get_ticks(void);
// Sleep with a busy loop
void sleep_busy(uint32_t milliseconds);
// Sleep with interrupts and hlt
void sleep_interrupt(uint32_t milliseconds);

#endif