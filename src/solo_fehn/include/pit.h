/*
 * pit.h - Programmable Interval Timer (8253/8254)
 *
 * The PIT is a tiny chip with three independent counters.  Counter 0 is
 * wired to IRQ0, so it is used to generate a periodic timer interrupt.
 * Counter 2 is wired to the PC speaker, so it can also be used to make
 * the speaker beep at a chosen frequency (we will use that in Assignment 5).
 *
 * The PIT runs from a fixed input clock of 1,193,180 Hz.  The frequency
 * of the timer interrupt is that input clock divided by a 16-bit value
 * we write to the chip.  We pick TARGET_FREQUENCY = 1000 Hz, so a tick
 * happens every millisecond and TICKS_PER_MS = 1.
 */

#pragma once
#include <libc/stdint.h>

#define PIT_CMD_PORT          0x43
#define PIT_CHANNEL0_PORT     0x40
#define PIT_CHANNEL1_PORT     0x41
#define PIT_CHANNEL2_PORT     0x42
#define PC_SPEAKER_PORT       0x61

#define PIT_BASE_FREQUENCY    1193180u
#define TARGET_FREQUENCY      1000u
#define DIVIDER               (PIT_BASE_FREQUENCY / TARGET_FREQUENCY)
#define TICKS_PER_MS          (TARGET_FREQUENCY / 1000u)

/* Initialise the PIT to fire IRQ0 every millisecond. */
void init_pit(void);

/* Sleep using busy-waiting (high CPU usage). */
void sleep_busy(uint32_t milliseconds);

/* Sleep using interrupts and HLT (low CPU usage). */
void sleep_interrupt(uint32_t milliseconds);

/* Number of timer ticks since boot. */
uint32_t get_current_tick(void);
