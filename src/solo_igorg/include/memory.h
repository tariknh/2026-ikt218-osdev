#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h>
#include <libc/stddef.h>

/*
 * Initializes kernel heap.
 */
void init_kernel_memory(uint32_t* kernel_end);

/*
 * Kernel heap allocator.
 */
void* malloc(size_t size);

/*
 * For later improvement.
 */
void free(void* pointer);

/*
 * Prints current memory layout and heap state.
 */
void print_memory_layout(void);

#endif
