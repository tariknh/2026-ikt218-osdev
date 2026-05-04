#include <libc/stdint.h>
#include <libc/stddef.h>
#include "kernel_memory.h"
#include "terminal.h"

#define KERNEL_HEAP_SIZE (1024 * 1024) // 1 MB heap size

typedef struct block_header {
    size_t size;
    uint8_t free;
    struct block_header* next;
} block_header_t;

static uint32_t heap_start_addr = 0;
static uint32_t heap_end_addr = 0;
static block_header_t* free_list = NULL;

static uint32_t align4(uint32_t addr) {
    return (addr + 3) & ~((uint32_t)3);
}

static size_t align_size(size_t size) {
    return (size + 3u) & ~((size_t)3u);
}

void init_kernel_memory(uint32_t* kernel_end) {
    heap_start_addr = align4((uint32_t)kernel_end);
    heap_end_addr = heap_start_addr + KERNEL_HEAP_SIZE;

    // Initialize the free list with a single large block
    free_list = (block_header_t*)heap_start_addr;
    free_list->size = KERNEL_HEAP_SIZE - sizeof(block_header_t);
    free_list->free = 1;
    free_list->next = NULL;
}

static void split_block(block_header_t* block, size_t size) {
    if (block->size <= size + sizeof(block_header_t)) {
        return; // Not enough space to split
    }

    // Create a new block header for the remaining space
    block_header_t* new_block = (block_header_t*)((uint32_t)block + sizeof(block_header_t) + size);
    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->free = 1;
    new_block->next = block->next;

    // Update the original block
    block->size = size;
    block->next = new_block;
}

void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size = align_size(size);

    block_header_t* current = free_list;

    while (current) {
        if (current->free && current->size >= size) {
            split_block(current, size);
            current->free = 0;
            return (void*)((uint32_t)current + sizeof(block_header_t));
        }
        current = current->next;
    }

    return NULL; // No suitable block found

}

static void merge_blocks() {
    block_header_t* current = free_list;

    while (current != NULL && current->next != NULL) {
        if (current->free && current->next->free) {
            // Merge the two blocks
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    block_header_t* block = (block_header_t*)((uint32_t)ptr - sizeof(block_header_t));
    block->free = 1;
    merge_blocks();
}


void print_memory_layout(void) {
    terminal_write("Kernel heap start: ");
    terminal_write_hex(heap_start_addr);
    terminal_write("\n");

    terminal_write("Kernel heap end: ");
    terminal_write_hex(heap_end_addr);
    terminal_write("\n");

    terminal_write("Free blocks:\n");

    block_header_t* current = free_list;
    int i = 0;

    while (current != NULL) {
        terminal_write("Block ");
        terminal_write_dec(i);

        terminal_write(": Address: ");
        terminal_write_hex((uint32_t)current);

        terminal_write(", Size: ");
        terminal_write_dec((uint32_t)current->size);
        terminal_write(" bytes");

        terminal_write(", Free: ");
        terminal_write(current->free ? "Yes" : "No");
        terminal_write("\n");

        current = current->next;
        i++;
    }
}