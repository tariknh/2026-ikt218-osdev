#include "memory.h"
#include "screen.h"
#include "libc/stdint.h"
#include "libc/stddef.h"

// Header stored before each memory block
typedef struct block_header {
    size_t size;                // Size of this block
    int free;                   // 1 if free, 0 if used
    struct block_header* next;  // Next block in the list
} block_header_t;

#define HEAP_SIZE_BYTES (1024 * 1024) // Heap size: 1 MB
#define ALIGN4(x) (((x) + 3) & ~3) // Aligns size to 4 bytes

static uint8_t* heap_start = 0; // Heap start address
static uint8_t* heap_end = 0; // Heap end address
static block_header_t* free_list = 0; // First block in the heap

void init_kernel_memory(uint32_t* kernel_end) {
    uint32_t start = (uint32_t)kernel_end;     // Start heap after the kernel
    start = (start + 0xFFF) & ~0xFFF;     // Align heap start to a 4 KB page
    
    // Set heap range
    heap_start = (uint8_t*)start;
    heap_end = heap_start + HEAP_SIZE_BYTES;

    // Create the first free block
    free_list = (block_header_t*)heap_start;
    free_list->size = HEAP_SIZE_BYTES - sizeof(block_header_t);
    free_list->free = 1;
    free_list->next = 0;
}

// Splits a large block into two smaller blocks
static void split_block(block_header_t* block, size_t size) {
    // Do not split if there is not enough space
    if (block->size <= size + sizeof(block_header_t)) {
        return;
    }

    block_header_t* new_block = (block_header_t*)((uint8_t*)(block + 1) + size);     // Create a new block after the allocated part
    // Set new block information
    new_block->size = block->size - size - sizeof(block_header_t);
    new_block->free = 1;
    new_block->next = block->next;
    // Shrink current block
    block->size = size;
    block->next = new_block;
}
// Allocates memory from the heap
void* malloc(size_t size) {
    // Reject empty allocation or uninitialized heap
    if (size == 0 || free_list == 0) {
        return 0;
    }

    size = ALIGN4(size);     // Align allocation size

    // Search for a free block
    block_header_t* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            split_block(current, size);
            current->free = 0;
            return (void*)(current + 1);             // Return memory after the header
        }
        current = current->next;
    }

    return 0;     // No suitable block found
}

// Merges neighboring free blocks
static void coalesce_free_blocks(void) {
    block_header_t* current = free_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            // Merge current block with next block
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// Frees allocated memory
void free(void* ptr) {
    // Ignore null pointer
    if (!ptr) {
        return;
    }
    block_header_t* block = ((block_header_t*)ptr) - 1;    // Get block header before the memory pointer
    block->free = 1;     // Mark block as free
    coalesce_free_blocks();     // Merge free blocks
}

// Prints heap information
void print_memory_layout(void) {
    screen_write("Kernel heap start: ");
    screen_write_hex((uint32_t)heap_start);
    screen_write("\nKernel heap end:   ");
    screen_write_hex((uint32_t)heap_end);
    screen_write("\nKernel heap size:  ");
    screen_write_dec(HEAP_SIZE_BYTES);
    screen_write(" bytes\n");
}