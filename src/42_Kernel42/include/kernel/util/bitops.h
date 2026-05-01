#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * Operations for bit operations, all decleared static inline to suggest to the compiler to inline
 * these for performance
 */

/**
 * Set a bit to 1
 * @param var variable to modify
 * @param bit bit to set
 */
inline __attribute__((always_inline)) void bit_set(uint8_t* var, uint8_t bit) {
  *var |= (uint8_t)(1u << bit);
}

/**
 * Set a bit to 0
 * @param var variable to modify
 * @param bit bit to set
 */
inline __attribute__((always_inline)) void bit_clear(uint8_t* var, uint8_t bit) {
  *var &= ~(uint8_t)(1u << bit);
}

/**
 * Flip a bit
 * @param var variable to modify
 * @param bit bit to flip
 */
inline __attribute__((always_inline)) void bit_toggle(uint8_t* var, uint8_t bit) {
  *var ^= (uint8_t)(1u << bit);
}

/**
 * Test a bit
 * @param var variable
 * @param bit bit to test
 * @return true if bit is 1, false if 0.
 */
inline __attribute__((always_inline)) bool bit_test(uint8_t var, uint8_t bit) {
  return (var & (1 << bit)) != 0;
}
