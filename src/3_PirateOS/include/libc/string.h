#pragma once
#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// Returns the length of a null-terminated string
size_t strlen(const char* str);

#ifdef __cplusplus
}
#endif
