#pragma once

#include "libc/stdint.h" // Includes fixed-size integer types

#define PIT_BASE_FREQUENCY 1193180 // Base frequency of the PIT hardware timer

void init_pit(void); // Initializes the Programmable Interval Timer
uint32_t pit_get_ticks(void); // Returns the number of timer ticks since startup
void pit_sleep_ms(uint32_t ms); // Sleeps for the given number of milliseconds
