#include "memory.h"

void* memcpy(void* dest, const void* src, uint32_t n)
{
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    uint32_t i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* dest, uint8_t val, uint32_t n)
{
    uint8_t* d = (uint8_t*)dest;
    uint32_t i;
    for (i = 0; i < n; i++) {
        d[i] = val;
    }
    return dest;
}

void* memset16(void* dest, uint16_t val, uint32_t n)
{
    uint16_t* d = (uint16_t*)dest;
    uint32_t i;
    for (i = 0; i < n / 2; i++) {
        d[i] = val;
    }
    return dest;
}
