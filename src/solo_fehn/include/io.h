/*
 * io.h - x86 port I/O wrappers
 *
 * The PC has a separate I/O address space (16-bit ports) accessed via the
 * IN and OUT instructions. Devices like the PIC, the keyboard controller,
 * and the PIT all live there.  These tiny inline wrappers let the rest of
 * the kernel use them like ordinary C functions.
 */

#pragma once
#include <libc/stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* Write a byte to the unused diagnostic port 0x80. Many devices need a few
   I/O cycles between commands; reading or writing 0x80 is the classic way
   to get one without doing anything observable. */
static inline void io_wait(void) {
    outb(0x80, 0);
}
