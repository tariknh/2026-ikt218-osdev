#include "kernel/memory.h"

void* memcpy(void* dest, const void* src, size_t count)
{
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (count & 1) { *d++ = *s++; count--; }
    count >>= 1;
    while (count--) { *d++ = *s++; *d++ = *s++; }
    return dest;
}

void* memset16(void* ptr, uint16_t value, size_t num)
{
    uint16_t* p = (uint16_t*)ptr;
    while (num--) *p++ = value;
    return ptr;
}

void* memset(void* ptr, int value, size_t num)
{
    unsigned char* p = (unsigned char*)ptr;
    while (num--) *p++ = (unsigned char)value;
    return ptr;
}
