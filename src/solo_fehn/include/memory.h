/*
 * memory.h - Kernel heap allocator
 *
 * The linker script defines the symbol "end" at the byte that comes right
 * after the kernel image in physical memory.  We use that as the start of
 * the kernel heap.  The allocator itself is a small linked-list of blocks
 * with a header that tells us how big each block is and whether it is
 * currently in use.
 */

#pragma once
#include <libc/stdint.h>
#include <libc/stddef.h>

/* The linker symbol marking the end of the kernel image.
   `&end` is the first byte the heap is allowed to use. */
extern uint32_t end;

/* Initialise the heap. Pass &end (or any later kernel-end pointer). */
void init_kernel_memory(uint32_t* kernel_end);

/* Allocate `size` bytes, aligned to 4 bytes. Returns NULL on failure. */
void* malloc(size_t size);

/* Free a previously malloc()'d pointer. NULL is allowed. */
void free(void* ptr);

/* Print the current state of the heap to the terminal. */
void print_memory_layout(void);
