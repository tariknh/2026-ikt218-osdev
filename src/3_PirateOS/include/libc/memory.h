#pragma once

#include "libc/stddef.h"
#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// Copies num bytes from src to dest
void *memcpy(void *dest, const void *src, size_t num);

// Fills num bytes with the same byte value
void *memset(void *ptr, int value, size_t num);

// Fills num 16-bit values with the same value
void *memset16(void *ptr, uint16_t value, size_t num);

#ifdef __cplusplus
}
#endif