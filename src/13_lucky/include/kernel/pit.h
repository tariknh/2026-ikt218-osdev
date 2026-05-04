#pragma once

#include "stdint.h"

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_BASE_FREQUENCY 1193180
#define PIT_TARGET_FREQUENCY 1000
#define PIT_DIVIDER (PIT_BASE_FREQUENCY / PIT_TARGET_FREQUENCY)
#define TICKS_PER_MS (PIT_TARGET_FREQUENCY / 1000)

void init_pit(void);

void pit_tick(void);

uint32_t get_current_tick(void);

void sleep_busy(uint32_t milliseconds);

void sleep_interrupt(uint32_t milliseconds);
