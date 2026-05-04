#pragma once

#include <libc/stdint.h>

#define PIT_BASE_FREQUENCY 1193182u
#define PIT_TARGET_FREQUENCY 1000u
#define PIT_DIVISOR (PIT_BASE_FREQUENCY / PIT_TARGET_FREQUENCY)

void init_pit(void);
void pit_on_irq0(void);
uint32_t pit_get_ticks(void);
void sleep_busy(uint32_t milliseconds);
void sleep_interrupt(uint32_t milliseconds);