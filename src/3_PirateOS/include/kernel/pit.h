#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Channel 0 is used by the kernel timer in init_pit().
 * Channel 2 is available for PC speaker sound generation in Assignment 5.
 */
#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PC_SPEAKER_PORT 0x61

#define PIC1_CMD_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIC_EOI 0x20

#define PIT_BASE_FREQUENCY 1193180
#define TARGET_FREQUENCY 1000
#define DIVIDER (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS 1

void init_pit(void);
void sleep_interrupt(uint32_t milliseconds);
void sleep_busy(uint32_t milliseconds);
uint32_t pit_get_ticks(void);

#ifdef __cplusplus
}
#endif

#endif
