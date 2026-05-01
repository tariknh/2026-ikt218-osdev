#ifndef UIAOS_KERNEL_PIT_H
#define UIAOS_KERNEL_PIT_H

#include <stdint.h>

#define PIT_CMD_PORT       0x43
#define PIT_CHANNEL0_PORT  0x40
#define PIT_CHANNEL1_PORT  0x41
#define PIT_CHANNEL2_PORT  0x42
#define PC_SPEAKER_PORT    0x61

#define PIT_BASE_FREQUENCY 1193180u
#define TARGET_FREQUENCY   1000u
#define PIT_DIVIDER        (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS       1u

#define PIC1_COMMAND_PORT  0x20
#define PIC1_DATA_PORT     0x21
#define PIC2_COMMAND_PORT  0xA0
#define PIC2_DATA_PORT     0xA1
#define PIC_EOI            0x20

void init_pit(void);
void sleep_interrupt(uint32_t milliseconds);
void sleep_busy(uint32_t milliseconds);
uint32_t pit_get_ticks(void);

#endif
