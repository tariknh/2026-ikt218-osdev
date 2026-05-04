#ifndef MEMORY_H
#define MEMORY_H

#include "libc/stdint.h" // Includes fixed-size integer types
#include "libc/stddef.h" // Includes size_t

void init_kernel_memory(uint32_t* kernel_end); // Initializes the kernel memory manager
void* malloc(size_t size); // Allocates memory from the heap
void free(void* ptr); // Frees allocated memory
void print_memory_layout(void); // Prints the current memory layout


#endif