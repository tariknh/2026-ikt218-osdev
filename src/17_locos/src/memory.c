#include "memory.h"
#include "terminal.h"

// Heap size for the kernel allocator
#define KERNEL_HEAP_SIZE (1024 * 1024)  /* 1 MiB */
// Keep blocks aligned for simple access
#define ALIGNMENT 8

// Header for each heap block
struct block_header {
    uint32_t size;
    uint32_t free;
    struct block_header *next;
};

// Start and end of the heap area
static struct block_header *heap_head = 0;
static uint32_t heap_start_addr = 0;
static uint32_t heap_end_addr = 0;

// Round a value up to the next aligned address
static uint32_t align_up(uint32_t value, uint32_t align) {
    return (value + (align - 1)) & ~(align - 1);
}

// Build one free heap block after the kernel image
void init_kernel_memory(uint32_t *kernel_end) {
    // Find the first usable heap address
    heap_start_addr = align_up((uint32_t)kernel_end, ALIGNMENT);
    heap_end_addr = heap_start_addr + KERNEL_HEAP_SIZE;

    // Create the first free block
    heap_head = (struct block_header *)heap_start_addr;
    heap_head->size = KERNEL_HEAP_SIZE - (uint32_t)sizeof(struct block_header);
    heap_head->free = 1;
    heap_head->next = 0;
}

// Simple first fit allocator
void *malloc(uint32_t size) {
    if (size == 0 || heap_head == 0) {
        return 0;
    }

    // Keep allocations aligned
    size = align_up(size, ALIGNMENT);

    // Walk the block list and find free space
    struct block_header *curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            // Split the block if there is enough room left
            uint32_t remaining = curr->size - size;
            if (remaining > sizeof(struct block_header) + ALIGNMENT) {
                struct block_header *split = (struct block_header *)((uint32_t)curr + sizeof(struct block_header) + size);
                split->size = remaining - (uint32_t)sizeof(struct block_header);
                split->free = 1;
                split->next = curr->next;
                curr->next = split;
                curr->size = size;
            }

            curr->free = 0;
            return (void *)((uint32_t)curr + sizeof(struct block_header));
        }
        curr = curr->next;
    }

    return 0;
}

// Mark a block as free and merge neighbours
void free(void *ptr) {
    if (ptr == 0) {
        return;
    }

    // Move back from the payload to the block header
    struct block_header *block = (struct block_header *)((uint32_t)ptr - sizeof(struct block_header));
    block->free = 1;

    // Merge adjacent free blocks when possible
    struct block_header *curr = heap_head;
    while (curr && curr->next) {
        uint32_t curr_end = (uint32_t)curr + sizeof(struct block_header) + curr->size;
        if (curr->free && curr->next->free && curr_end == (uint32_t)curr->next) {
            curr->size += (uint32_t)sizeof(struct block_header) + curr->next->size;
            curr->next = curr->next->next;
            continue;
        }
        curr = curr->next;
    }
}

// Print the heap range for debugging
void print_memory_layout(void) {
    terminal_printf("Kernel heap: 0x%x - 0x%x (%u KiB)\n",
                    heap_start_addr,
                    heap_end_addr,
                    (KERNEL_HEAP_SIZE / 1024));
}
