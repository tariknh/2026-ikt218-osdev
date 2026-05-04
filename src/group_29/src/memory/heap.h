#pragma once

#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"

#include "../printing/printing.h"
#include "../vga_text_mode_interface/vga_text_mode_interface.h"

#include "memory_macros.h"


/**
 * \file
 * Fixed-size early kernel heap placed directly after the linker-provided end
 * symbol. The allocator uses a first-fit free list, splits large free blocks
 * during allocation, and merges adjacent free blocks during free.
 */

/**
 * \brief Metadata stored in front of every heap allocation.
 */
struct heap_block {
    /** \brief Number of usable bytes in this block. */
    size_t size;
    /** \brief Whether the block is currently available for reuse. */
    bool is_free;
    /** \brief Pointer to the next block in the heap list. */
    struct heap_block* next;
};

typedef struct {
    uint32_t kernel_end;
    uint32_t heap_start;
    uint32_t heap_end;
    char formatted[80];
} MemoryDebugData;

typedef struct {
    // data
    size_t total_payload_bytes;
    size_t total_bytes_with_meta;

    size_t used_bytes;
    size_t free_bytes;

    size_t used_bytes_with_meta;
    size_t free_bytes_with_meta;

    // counts
    size_t total_blocks;
    size_t used_blocks;
    size_t free_blocks;

    // fragmentation
    size_t largest_free_block;
    size_t smallest_free_block;

    // fragmentation ratio
    size_t fragmentation_per_mille;

    // allocation
    size_t largest_used_block;
    size_t smallest_used_block;

    // averages
    size_t avg_free_block_size;
    size_t avg_used_block_size;
} HeapInformation;

/** \brief Initializes the kernel heap at the first aligned address after kernel_end. */
void init_kernel_memory(uint32_t* kernel_end);

/** \brief Initializes a simple identity-mapped paging setup for the kernel. */
void init_paging(void);

/** \brief Maps a virtual page to a physical page inside the first 4 MiB. */
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

/** \brief Allocates memory and returns a page-aligned pointer. */
char* pmalloc(size_t size);

/** \brief Allocates at least size bytes from the kernel heap. */
void* malloc(size_t size);

/** \brief Frees a block previously returned by malloc. */
void free(void* ptr);

/** \brief Prints the kernel end, heap start, and heap end addresses to VGA. */
void print_memory_layout(void);

MemoryDebugData get_memory_layout(void);

HeapInformation get_heap_stats(void);