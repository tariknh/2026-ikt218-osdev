#ifndef IO_H
#define IO_H

// Instead of typedefs or <stdint.h>, use your local project version
#include "../include/libc/stdint.h"

// This tells the compiler how to talk to the hardware ports
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);

#endif