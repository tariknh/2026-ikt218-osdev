#include "ports.h"

// Reads one byte from an I/O port
uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));     // Input byte from port into AL
    return result;
}

// Writes one byte to an I/O port
void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));     // Output byte from AL to port

}