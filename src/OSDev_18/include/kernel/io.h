#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <libc/stdint.h>

// Writes a byte to a port using outb instruction
// The value is placed in the AL register (a)
// The port number is provided as an immediate value or placed in DX register (Nd)
// Used to send commands or data directly to hardware devices
static inline void OutPortByte(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Reads a byte from a port using inb instruction
// The port number is passed as an immediate value or DX register (Nd)
// The value is returned from the AL register (a)
// Used to receive data from hardware devices
static inline uint8_t InPortByte(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Performs a short delay by writing to port 0x80 which is traditionally unused
// Used to ensure that hardware devices have time to process previous commands
static inline void IoWait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif