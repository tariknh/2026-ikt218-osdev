#pragma once
#include <stdbool.h>
#include <stdint.h>

// NOTE: This file is sort of a scratch file, so most things put here will eventuyallyally be
// refactored aw


// K&R
/**
 * Convert a number to its ascii representation
 * @param n integer number
 * @param str variable to store string in
 */
void int_to_ascii(int n, char str[]);


/**
 * Read the timestamp counter
 * @return the number of CPU cycles since its reset
 */
static inline uint64_t rdtsc(void) {
  uint32_t lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi) : :);
  return ((uint64_t)hi << 32) | lo;
}

/**
 * Helper for dividing a 64 bit number by 10
 * @param n number to divide
 * @returns result
 */
static inline uint32_t div_u64_by_10(uint64_t* n) {
  uint32_t high = (*n >> 32);
  uint32_t low = (*n & 0xFFFFFFFF);
  uint64_t q_high = (high * 0xCCCCCCCDULL) >> 35;
  uint64_t rem_high = high - q_high * 10;

  uint64_t full = (rem_high << 32) | low;
  uint64_t q_low = (full * 0xCCCCCCCDULL) >> 35;

  *n = (q_high << 32) | q_low;
  return full - q_low * 10;
}

// Should only be called with compatible types
#define CLAMP(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))
