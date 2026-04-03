#ifndef PIT_H
#define PIT_H

#include <libc/stdint.h>
#include <libc/stdbool.h>

/* PIT (Programmable Interval Timer) ports */
#define PIT_CMD_PORT        0x43
#define PIT_CHANNEL0_PORT   0x40
#define PIT_CHANNEL1_PORT   0x41
#define PIT_CHANNEL2_PORT   0x42
#define PC_SPEAKER_PORT     0x61
#define PIT_DEFAULT_DIVISOR 0x4E20  /* 20000 -> ~59.66 Hz */

/* IRQ0 / PIC */
#define PIC1_CMD_PORT   0x20
#define PIC1_DATA_PORT  0x20
#define PIC_EOI         0x20

/* Frequency / timing constants */
#define PIT_BASE_FREQUENCY  1193180
#define TARGET_FREQUENCY    1000        /* 1000 Hz = 1 tick per ms */
#define DIVIDER             (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS        (TARGET_FREQUENCY / TARGET_FREQUENCY)  /* = 1 */

void init_pit(void);
void pit_handler(void);           /* called from irq_handler on IRQ0 */
uint32_t get_current_tick(void);
void sleep_interrupt(uint32_t milliseconds);
void sleep_busy(uint32_t milliseconds);

#endif /* PIT_H */
