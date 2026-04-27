#pragma once
#include <stdbool.h>
#include <stdint.h>

/*
 * PIT (Programmable Interval Timer) driver.
 *
 * Channel 0 is configured to generate IRQ0 at TARGET_FREQUENCY Hz,
 * which is converted into a tick counter used by sleep and time.
 * Each tick corresponds to TICKS_PER_MS milliseconds.
 *
 * PIT base frequency: PIT_BASE_FREQUENCY = 1193182 Hz (8254 crystal).
 */

/**
 * Initialize the programmable interval timer
 */
void init_pit();

/**
 * Sleep using interrupts to wake up
 * @param milliseconds time to sleep for
 */
void sleep_interrupt(uint32_t milliseconds);

/**
 * Sleep by constantly checking for time left
 * @param milliseconds time to sleep for
 */
void sleep_busy(uint32_t milliseconds);
