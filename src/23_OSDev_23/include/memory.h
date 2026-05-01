#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// This symbol is defined in arch/i386/linker.ld — it marks the end of the kernel image
extern uint32_t end;

// Initialize the kernel heap starting just after the kernel image
void init_kernel_memory(uint32_t* kernel_end);

// Allocate 'size' bytes from the bump heap (never freed)
void* malloc(uint32_t size);

// No-op for a simple bump allocator
void free(void* ptr);

// Print the current memory layout to the terminal
void print_memory_layout();

#endif /* MEMORY_H */
