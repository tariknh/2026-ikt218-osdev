#pragma once
#include "../../include/libc/stdint.h"

/* inline means that a function call can be replaced by the compiler by just
having the code of the function be pasted directly into the place where it's
called from. This is better for performance */

/// @brief Output byte to io port
/// @param port 
/// @param value 
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

/// @brief Read byte from io port
/// @param port 
/// @return 
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/// @brief Output word to io port
/// @param port 
/// @param value 
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

/// @brief Read word from io port
/// @param port 
/// @return 
static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ __volatile__("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}