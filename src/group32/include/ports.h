#ifndef PORTS_H
#define PORTS_H

#include "libc/stdint.h" // Includes fixed-size integer types

uint8_t inb(uint16_t port); // Reads one byte from an I/O port
void outb(uint16_t port, uint8_t value); // Writes one byte to an I/O port

#endif