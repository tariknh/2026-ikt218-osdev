#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <libc/stdint.h>
#include <libc/stdbool.h>

#define PIT_CMD_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PC_SPEAKER_PORT 0x61
#define PIT_DEFAULT_DIVISOR 0x4E20 //20000, gets just shy of 60Hz
#define PIC1_CMD_PORT 0x20
#define PIC1_DATA_PORT 0x21
#define PIT_BASE_FREQ 1193180
#define TARGET_FREQ 1000
#define DIVIDER (PIT_BASE_FREQ / TARGET_FREQ)
#define TICKS_PER_MS (TARGET_FREQ / TARGET_FREQ) // = 1, needed for converting ms into ticks

uint32_t GetCurrentTick(void);

void PitInitialize();
void SleepInterrupt(uint32_t ticks);
void SleepBusy(uint32_t milliseconds);
void SleepTest();

#endif