#include "memory.h"

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t memory_used = 0;

// Initialize the kernel memory manager
void init_kernel_memory(uint32_t* kernel_end)
{
    last_alloc = (uint32_t)kernel_end + 0x1000;
    heap_begin = last_alloc;
    heap_end = 0x400000;
    memory_used = 0;
}

// Allocate a block of memory
void* malloc(size_t size)
{
    if (!size) return 0;

    // Loop through blocks to find an available block with enough size
    uint8_t *mem = (uint8_t *)heap_begin;

    while ((uint32_t)mem < last_alloc)
    {
        alloc_t *a = (alloc_t *)mem;

        // If this block has no size, stop searching
        if (!a->size) {
            break;
        }

        // If the block is already used, skip it
        if (a->status) {
            mem += a->size;
            mem += sizeof(alloc_t);
            continue;
        }

        // If the block is free and large enough, reuse it
        if (a->size >= size)
        {
            a->status = 1;
            memory_used += size + sizeof(alloc_t);
            return (void *)(mem + sizeof(alloc_t));
        }

        // If the free block is too small, skip it
        mem += a->size;
        mem += sizeof(alloc_t);
    }

    // If no free memory block is found, allocate a new memory block on the heap
    alloc_t *alloc = (alloc_t *)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    // Point to the usable memory area after the allocation header
    void *allocated_memory = (void *)(last_alloc + sizeof(alloc_t));

    // Move last_alloc past the header and the requested memory block
    last_alloc += sizeof(alloc_t);
    last_alloc += size;

    // Track total memory used by the header and the requested memory block
    memory_used += sizeof(alloc_t);
    memory_used += size;

    return allocated_memory;
}

// Free a block of memory
void free(void *mem)
{
    if (!mem) {
        return;
    }

    // Move pointer back to the allocation header and mark the block as free
    alloc_t *alloc = (alloc_t *)((uint32_t)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

// Print a 32-bit value as hexadecimal
static void print_hex(uint32_t value)
{
    const char* digits = "0123456789ABCDEF";

    terminal_write("0x");

    for (int i = 28; i >= 0; i -= 4) {
        uint8_t digit = (value >> i) & 0xF;
        terminal_putchar(digits[digit]);
    }
}

// Print current memory layout
void print_memory_layout()
{
    terminal_write("Memory layout:\n");

    terminal_write("Heap start: ");
    print_hex(heap_begin);
    terminal_write("\n");

    terminal_write("Heap end: ");
    print_hex(heap_end);
    terminal_write("\n");

    terminal_write("Last allocation: ");
    print_hex(last_alloc);
    terminal_write("\n");

    terminal_write("Memory used: ");
    print_hex(memory_used);
    terminal_write("\n");
}