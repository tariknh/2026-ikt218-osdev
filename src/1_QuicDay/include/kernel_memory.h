
#pragma once

#include "libc/stdint.h"

void init_kernel_memory(void* kernel_end);
void* malloc(size_t size);
void free(void* ptr);

void init_paging(void);
void print_memory_layout(void);
