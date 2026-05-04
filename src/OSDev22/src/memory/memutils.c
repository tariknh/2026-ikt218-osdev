#include "memory.h"
#include "libc/stdint.h"
#include "libc/stddef.h"

/* Copy memory from source to destination */
void* memcpy(void* dest, const void* src, size_t count)
{
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;

    if (count & 1) {
        dst8[0] = src8[0];
        dst8 += 1;
        src8 += 1;
    }

    count /= 2;
    while (count--) {
        dst8[0] = src8[0];
        dst8[1] = src8[1];

        dst8 += 2;
        src8 += 2;
    }

    return dest;
}

/* Set memory using a 16-bit value */
void* memset16(void* ptr, uint16_t value, size_t num)
{
    uint16_t* p = (uint16_t*)ptr;
    while (num--)
        *p++ = value;

    return ptr;
}