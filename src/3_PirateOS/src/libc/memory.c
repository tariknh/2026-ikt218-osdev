#include "libc/memory.h"

// Copies bytes from src to dest
void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *dst8 = (uint8_t *)dest;
    const uint8_t *src8 = (const uint8_t *)src;

    while (count--) {
        *dst8++ = *src8++;
    }

    return dest;
}

// Fills memory with a repeated 16-bit value
void *memset16(void *ptr, uint16_t value, size_t num)
{
    uint16_t *p = (uint16_t *)ptr;

    while (num--) {
        *p++ = value;
    }

    return ptr;
}

// Fills memory with a repeated byte value
void *memset(void *ptr, int value, size_t num)
{
    uint8_t *p = (uint8_t *)ptr;

    while (num--) {
        *p++ = (uint8_t)value;
    }

    return ptr;
}