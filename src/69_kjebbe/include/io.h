#pragma once

#include "libc/stdint.h"

// Write a byte to an I/O port
static inline void outb(uint16_t port, uint8_t value) {
  asm volatile("out %1, %0" : : "dN"(port), "a"(value));
}

// Read a byte from an I/O port
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("in %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}
