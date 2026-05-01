#ifndef IO_H
#define IO_H

#include <libc/stdint.h>

// Write one byte to an I/O port
static inline void outb(uint16_t port, uint8_t value){
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read one byte from an I/O port
static inline uint8_t inb(uint16_t port){
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Small delay for I/O operations
static inline void io_wait(void){
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

#endif