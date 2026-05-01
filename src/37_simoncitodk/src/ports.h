#ifndef PORTS_H
#define PORTS_H

#include <libc/stdint.h>

/*
 * Read one byte from an x86 I/O port.
 *
 * The "Nd" constraint allows an immediate port number when possible,
 * or the DX register when needed.
 */
static inline uint8_t port_byte_in(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/*
 * Write one byte to an x86 I/O port.
 */
static inline void port_byte_out(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

#endif
