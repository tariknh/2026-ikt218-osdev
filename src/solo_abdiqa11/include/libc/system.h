#ifndef LIBC_SYSTEM_H
#define LIBC_SYSTEM_H

#include <stdint.h>
#include <stddef.h>

/* Memory helpers */
void* memset(void* dst, int v, size_t n);
void* memcpy(void* dst, const void* src, size_t n);

/* Port I/O */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
