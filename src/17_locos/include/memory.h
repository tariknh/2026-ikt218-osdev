#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h>

// Set up the kernel heap
void init_kernel_memory(uint32_t *kernel_end);
// Allocate one block from the heap
void *malloc(uint32_t size);
// Free one block back to the heap
void free(void *ptr);
// Print the heap layout for debugging
void print_memory_layout(void);

#endif
