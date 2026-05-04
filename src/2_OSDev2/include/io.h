#pragma once
#include <libc/stdint.h>

static inline void outb(uint16_t port, uint8_t value) { // Write a byte to the specified port
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {  // Read a byte from the specified port
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait() {  // Wait for an I/O operation to complete by writing to an unused port
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}