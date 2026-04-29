#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PC_SPEAKER_PORT 0x61

#define PIT_BASE_FREQUENCY 1193180
#define TARGET_FREQUENCY 1000
#define PIT_DIVIDER (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS 1

void init_pit(void);
void pit_handle_tick(void);
uint32_t pit_get_ticks(void);

void sleep_busy(uint32_t milliseconds);
void sleep_interrupt(uint32_t milliseconds);

#endif
