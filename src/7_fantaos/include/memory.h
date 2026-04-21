#pragma once

#include <libc/stdint.h>
#include <libc/stddef.h>

void init_kernel_memory(uint32_t *kernel_end);
void init_paging(void);
void *malloc(uint32_t size);
void free(void *ptr);
void print_memory_layout(void);
