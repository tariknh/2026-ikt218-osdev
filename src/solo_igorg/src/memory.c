#include <memory.h>
#include <terminal.h>
#include <libc/stdint.h>
#include <libc/stddef.h>

/*
 * Bump allocator.
 */
static uint32_t kernel_end_address = 0;
static uint32_t heap_start_address = 0;
static uint32_t heap_current_address = 0;
static uint32_t heap_used_bytes = 0;

#define HEAP_ALIGNMENT 16

static uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void init_kernel_memory(uint32_t* kernel_end)
{
    kernel_end_address = (uint32_t)kernel_end;

    /*
     * Starts heap at the next aligned address after kernel.
     */
    heap_start_address = align_up(kernel_end_address, HEAP_ALIGNMENT);
    heap_current_address = heap_start_address;
    heap_used_bytes = 0;
}

void* malloc(size_t size)
{
    if (size == 0) {
        return 0;
    }

    /*
     * Aligns allocation size to keep returned addresses properly aligned.
     */
    uint32_t aligned_size = align_up((uint32_t)size, HEAP_ALIGNMENT);
    uint32_t allocated_address = heap_current_address;

    heap_current_address += aligned_size;
    heap_used_bytes += aligned_size;

    return (void*)allocated_address;
}

void free(void* pointer)
{
    (void)pointer;
}

void print_memory_layout(void)
{
    terminal_write("  Kernel end: ");
    terminal_write_hex(kernel_end_address);
    terminal_putchar('\n');

    terminal_write("  Heap start: ");
    terminal_write_hex(heap_start_address);
    terminal_putchar('\n');

    terminal_write("  Heap current: ");
    terminal_write_hex(heap_current_address);
    terminal_putchar('\n');

    terminal_write("  Heap used: ");
    terminal_write_dec(heap_used_bytes);
    terminal_write(" bytes\n");
}
