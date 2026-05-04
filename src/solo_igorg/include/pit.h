#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>

/*
 * PIT I/O ports.
 */
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PIT_COMMAND_PORT  0x43

#define PIT_BASE_FREQUENCY 1193182

/*
 * We configure PIT to 1000 Hz.
 * This gives approximately one interrupt per millisecond.
 */
#define PIT_TARGET_FREQUENCY 1000

void init_pit(void);
void pit_handle_irq(void);

uint32_t pit_get_ticks(void);

void sleep_busy(uint32_t milliseconds);
void sleep_interrupt(uint32_t milliseconds);

#endif
