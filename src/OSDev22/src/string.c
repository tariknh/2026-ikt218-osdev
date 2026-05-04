#include "libc/string.h"

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

/* Setter 'count' bytes til verdien 'val' fra 'dest' */
void* memset(void* dest, int val, size_t count)
{
    uint8_t* p = (uint8_t*)dest;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)val;
    }
    return dest;
}