#pragma once

#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"

#include "../vga_text_mode_interface/vga_text_mode_interface.h"

#define HEAP_ALIGNMENT 8U
#define HEAP_SIZE_BYTES 0x100000U
#define PAGE_SIZE_BYTES 0x1000U
#define VGA_WIDTH 80U
#define VGA_MEMORY ((uint16_t*)0xB8000)

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

typedef struct {
    uint32_t kernel_end;
    uint32_t heap_start;
    uint32_t heap_end;
    char formatted[80];
} MemoryDebugData;

/* Initializes the kernel heap at the first aligned address after kernel_end. */
void init_kernel_memory(uint32_t* kernel_end);

/* Initializes a simple identity-mapped paging setup for the kernel. */
void init_paging(void);

/* Maps a virtual page to a physical page inside the first 4 MiB. */
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

/* Allocates memory and returns a page-aligned pointer. */
char* pmalloc(size_t size);

/* Allocates at least size bytes from the kernel heap. */
void* malloc(size_t size);

/* Frees a block previously returned by malloc. */
void free(void* ptr);

/* Prints the kernel end, heap start, and heap end addresses to VGA. */
void print_memory_layout(void);

MemoryDebugData get_memory_layout(void);
