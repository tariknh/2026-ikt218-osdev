#include <memory.h>
#include <terminal.h>

// Heap state, set by init_kernel_memory, advanced by malloc.
static uint32_t heap_start   = 0;
static uint32_t heap_current = 0;

// Page directory and one page table covering the first 4 MB.
// Must be 4 KB-aligned so CR3 can point directly at page_directory.
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_table_0[1024]   __attribute__((aligned(4096)));

// Round up to the next 4-byte boundary.
static uint32_t align4(uint32_t addr) {
    return (addr + 3) & ~3u;
}

void init_kernel_memory(uint32_t *kernel_end) {
    heap_start   = align4((uint32_t)kernel_end);
    heap_current = heap_start;
}

void init_paging(void) {
    // Identity-map the first 4 MB: virtual address == physical address.
    // Each entry maps one 4 KB page; flags 0x3 = present + read/write.
    for (uint32_t i = 0; i < 1024; i++)
        page_table_0[i] = (i * 0x1000) | 0x3;

    // Point directory entry 0 at the page table covering 0x0–0x3FFFFF.
    // All other entries left at 0 (not present).
    for (uint32_t i = 0; i < 1024; i++)
        page_directory[i] = 0;
    page_directory[0] = (uint32_t)page_table_0 | 0x3;

    // Load CR3 with the page directory base address.
    asm volatile("mov %0, %%cr3" : : "r"(page_directory) : "memory");

    // Set CR0.PG (bit 31) to enable paging.
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    asm volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}

// Bump allocator: advance the heap pointer by size bytes (4-byte aligned).
// Returns NULL if called before init_kernel_memory.
void *malloc(uint32_t size) {
    if (heap_start == 0 || size == 0)
        return (void *)0;

    void *ptr    = (void *)heap_current;
    heap_current = align4(heap_current + size);
    return ptr;
}

// Bump allocator has no per-allocation metadata, so free is a no-op.
void free(void *ptr) {
    (void)ptr;
}

void print_memory_layout(void) {
    printf("Memory layout:\n");
    printf("  Kernel start : 0x%x\n", 0x100000);
    printf("  Kernel end   : 0x%x\n", heap_start);
    printf("  Heap start   : 0x%x\n", heap_start);
    printf("  Heap current : 0x%x\n", heap_current);
}
