#include "arch/i386/cpu/ports.h"

#include <stdint.h>


// From https://github.com/cfenollosa/os-tutorial/blob/master/23-fixes/cpu/ports.c
uint8_t port_byte_in(uint16_t port) {
  uint8_t result;
  __asm__ volatile("in %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
  __asm__ volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

uint16_t port_word_in(uint16_t port) {
  uint16_t result;
  __asm__ volatile("in %%dx, %%ax" : "=a"(result) : "d"(port));
  return result;
}

void port_word_out(uint16_t port, uint16_t data) {
  __asm__ volatile("out %%ax, %%dx" : : "a"(data), "d"(port));
}
