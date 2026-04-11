#pragma once

#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"

/*
 * Fixed-size early kernel heap placed directly after the linker-provided end
 * symbol. The allocator uses a first-fit free list, splits large free blocks
 * during allocation, and merges adjacent free blocks during free.
 */

/*
 * Metadata stored in front of every heap allocation.
 *
 * size    Number of usable bytes in this block.
 * is_free Whether the block is currently available for reuse.
 * next    Pointer to the next block in the heap list.
 */
struct heap_block {
    size_t size;
    bool is_free;
    struct heap_block* next;
};

/* Initializes the kernel heap at the first aligned address after kernel_end. */
void init_kernel_memory(void* kernel_end);

/* Allocates at least size bytes from the kernel heap. */
void* malloc(size_t size);

/* Frees a block previously returned by malloc. */
void free(void* ptr);

/* Prints the kernel end, heap start, and heap end addresses to VGA. */
void print_memory_layout(void);
