#include "memory.h"
#include "terminal.h"
#include "libc/stdint.h"
#include "libc/stddef.h"

#define MAX_PAGE_ALIGNED_ALLOCS 32

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t *pheap_desc = 0;
uint32_t memory_used = 0;

void panic(const char* msg) {
    terminal_print_string(msg);
    while (1) { __asm__ __volatile__("hlt"); }
}

// Set up the kernel's memory system
void init_kernel_memory(uint32_t* kernel_end)
{
    last_alloc = (uint32_t)kernel_end + 0x1000;
    heap_begin = last_alloc;
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end = pheap_begin;
    memset((char *)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS);
    terminal_print_string("Kernel heap starts at 0x");
    uint32_t addr = last_alloc;
    char buf[9];
    buf[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = addr & 0xF;
        buf[i] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        addr >>= 4;
    }
    terminal_print_string(buf);
    terminal_print_string("\n");
}


void print_memory_layout()
{
    terminal_print_string("--- Memory Layout ---\n");

    // Memory used
    terminal_print_string("Memory used: ");
    uint32_t val = memory_used;
    char num[12];
    int i = 10;
    num[11] = '\0';
    num[i] = '\0';
    if (val == 0) { num[--i] = '0'; }
    else { while (val > 0) { num[--i] = '0' + (val % 10); val /= 10; } }
    terminal_print_string(&num[i]);
    terminal_print_string(" bytes\n");

    // Memory free
    terminal_print_string("Memory free: ");
    val = heap_end - heap_begin - memory_used;
    i = 10;
    if (val == 0) { num[--i] = '0'; }
    else { while (val > 0) { num[--i] = '0' + (val % 10); val /= 10; } }
    terminal_print_string(&num[i]);
    terminal_print_string(" bytes\n");

    // Heap size
    terminal_print_string("Heap size: ");
    val = heap_end - heap_begin;
    i = 10;
    if (val == 0) { num[--i] = '0'; }
    else { while (val > 0) { num[--i] = '0' + (val % 10); val /= 10; } }
    terminal_print_string(&num[i]);
    terminal_print_string(" bytes\n");

    terminal_print_string("Heap start: 0x");
    uint32_t addr = heap_begin;
    char hex[9]; hex[8] = '\0';
    for (int j = 7; j >= 0; j--) {
        uint8_t nibble = addr & 0xF;
        hex[j] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        addr >>= 4;
    }
    terminal_print_string(hex);
    terminal_print_string("\n");

    terminal_print_string("Heap end:   0x");
    addr = heap_end;
    for (int j = 7; j >= 0; j--) {
        uint8_t nibble = addr & 0xF;
        hex[j] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        addr >>= 4;
    }
    terminal_print_string(hex);
    terminal_print_string("\n");

    terminal_print_string("PHeap start: 0x");
    addr = pheap_begin;
    for (int j = 7; j >= 0; j--) {
        uint8_t nibble = addr & 0xF;
        hex[j] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        addr >>= 4;
    }
    terminal_print_string(hex);
    terminal_print_string("\n");

    terminal_print_string("PHeap end:   0x");
    addr = pheap_end;
    for (int j = 7; j >= 0; j--) {
        uint8_t nibble = addr & 0xF;
        hex[j] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        addr >>= 4;
    }
    terminal_print_string(hex);
    terminal_print_string("\n");

    terminal_print_string("---------------------\n");
}

void free(void *mem)
{
    alloc_t *alloc = (alloc_t *)((uint32_t)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

void pfree(void *mem)
{
    if ((uint32_t)mem < pheap_begin || (uint32_t)mem > pheap_end) return;
    uint32_t ad = (uint32_t)mem;
    ad -= pheap_begin;
    ad /= 4096;
    pheap_desc[ad] = 0;
}

char* pmalloc(size_t size)
{
    for (int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++)
    {
        if (pheap_desc[i]) continue;
        pheap_desc[i] = 1;
        return (char *)(pheap_begin + i * 4096);
    }
    panic("pmalloc was unable to allocate memory\n");
    return 0;
}

void* malloc(size_t size)
{
    if (!size) return 0;
    uint8_t *mem = (uint8_t *)heap_begin;
    while ((uint32_t)mem < last_alloc)
    {
        alloc_t *a = (alloc_t *)mem;
        if (!a->size)
            goto nalloc;
        if (a->status)
        {
            mem += a->size + sizeof(alloc_t) + 4;
            continue;
        }
        if (a->size >= size)
        {
            a->status = 1;
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (char *)(mem + sizeof(alloc_t));
        }
        mem += a->size + sizeof(alloc_t) + 4;
    }
nalloc:
    if (last_alloc + size + sizeof(alloc_t) >= heap_end)
    {
        panic("Memory limit reached\n");
    }
    alloc_t *alloc = (alloc_t *)last_alloc;
    alloc->status = 1;
    alloc->size = size;
    last_alloc += size + sizeof(alloc_t) + 4;
    memory_used += size + 4 + sizeof(alloc_t);
    memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char *)((uint32_t)alloc + sizeof(alloc_t));
}