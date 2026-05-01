#include "memory.h"
#include "terminal.h"

#define HEAP_SIZE_BYTES 0x100000
#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

static uint32_t heap_start = 0;
static uint32_t heap_current = 0;
static uint32_t heap_end = 0;
static uint32_t memory_used = 0;

static uint32_t *page_directory = 0;
static uint32_t page_directory_address = 0;
static uint32_t *next_page_table = 0;

static uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static uint32_t allocation_total_size(uint32_t size)
{
    return align_up(sizeof(alloc_t) + size, 4);
}

void init_kernel_memory(uint32_t *kernel_end)
{
    heap_start = align_up((uint32_t)kernel_end, 4);
    heap_current = heap_start;
    heap_end = heap_start + HEAP_SIZE_BYTES;
    memory_used = 0;
}

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t page_directory_index = virt >> 22;

    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        next_page_table[i] = phys | 3;
        phys += PAGE_SIZE;
    }

    page_directory[page_directory_index] = ((uint32_t)next_page_table) | 3;
    next_page_table = (uint32_t *)((uint32_t)next_page_table + PAGE_SIZE);
}

void init_paging(void)
{
    page_directory = (uint32_t *)0x400000;
    page_directory_address = (uint32_t)page_directory;
    next_page_table = (uint32_t *)0x404000;

    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        page_directory[i] = 0 | 2;
    }

    paging_map_virtual_to_phys(0x00000000, 0x00000000);
    paging_map_virtual_to_phys(0x00400000, 0x00400000);

    asm volatile("mov %0, %%cr3" : : "r"(page_directory_address));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    terminal_write("Paging enabled\n");
}

void print_memory_layout(void)
{
    terminal_write("Memory layout\n");

    terminal_write("Heap start: ");
    terminal_write_hex(heap_start);
    terminal_write("\n");

    terminal_write("Heap current: ");
    terminal_write_hex(heap_current);
    terminal_write("\n");

    terminal_write("Heap end: ");
    terminal_write_hex(heap_end);
    terminal_write("\n");

    terminal_write("Memory used: ");
    terminal_write_dec(memory_used);
    terminal_write(" bytes\n");
}

void *malloc(size_t size)
{
    if (size == 0) {
        return 0;
    }

    uint32_t requested_size = (uint32_t)size;
    uint32_t scan = heap_start;

    while (scan < heap_current) {
        alloc_t *existing = (alloc_t *)scan;
        uint32_t existing_total_size = allocation_total_size(existing->size);

        if (existing->status == 0 && existing->size >= requested_size) {
            existing->status = 1;
            memory_used += existing_total_size;
            return (void *)(scan + sizeof(alloc_t));
        }

        scan += existing_total_size;
    }

    uint32_t allocation_start = align_up(heap_current, 4);
    uint32_t allocation_size = allocation_total_size(requested_size);
    uint32_t allocation_end = allocation_start + allocation_size;

    if (allocation_end >= heap_end) {
        terminal_write("malloc failed: out of heap memory\n");
        return 0;
    }

    alloc_t *header = (alloc_t *)allocation_start;
    header->status = 1;
    header->size = requested_size;

    heap_current = allocation_end;
    memory_used += allocation_size;

    return (void *)(allocation_start + sizeof(alloc_t));
}

void free(void *mem)
{
    if (mem == 0) {
        return;
    }

    alloc_t *header = (alloc_t *)((uint32_t)mem - sizeof(alloc_t));

    if (header->status == 0) {
        return;
    }

    header->status = 0;
    memory_used -= allocation_total_size(header->size);
}
