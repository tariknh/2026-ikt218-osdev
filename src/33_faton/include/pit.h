#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>

#define PIT_CMD_PORT      0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIC1_CMD_PORT     0x20
#define PIC_EOI           0x20

#define PIT_BASE_FREQUENCY 1193180
#define TARGET_FREQUENCY   1000
#define DIVIDER            (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS       (TARGET_FREQUENCY / TARGET_FREQUENCY)

void     init_pit(void);
uint32_t get_current_tick(void);
void     sleep_busy(uint32_t milliseconds);
void     sleep_interrupt(uint32_t milliseconds);

#endif