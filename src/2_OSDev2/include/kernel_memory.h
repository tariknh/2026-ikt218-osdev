#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <libc/stdint.h>
#include <libc/stddef.h>

void init_kernel_memory(uint32_t* kernel_end);
void* malloc(size_t size);
void free(void* ptr);
void print_memory_layout(void);

#endif // KERNEL_MEMORY_H