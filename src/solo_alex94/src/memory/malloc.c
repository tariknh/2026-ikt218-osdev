#include "kernel/memory.h"
#include "terminal.h"
#include <libc/stdint.h>
#include <libc/stddef.h>

#define MAX_PAGE_ALIGNED_ALLOCS 32

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t* pheap_desc = 0;
uint32_t memory_used = 0;

static void kprint_dec(uint32_t value) {
    char buffer[16];
    int i = 0;

    if (value == 0) {
        terminal_write("0");
        return;
    }

    while (value > 0) {
        buffer[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        char out[2];
        out[0] = buffer[--i];
        out[1] = '\0';
        terminal_write(out);
    }
}

static void kprint_hex(uint32_t value) {
    char buffer[9];
    const char* hex = "0123456789ABCDEF";
    int started = 0;
    int pos = 0;

    terminal_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t digit = (uint8_t)((value >> shift) & 0xF);
        if (digit != 0 || started || shift == 0) {
            buffer[pos++] = hex[digit];
            started = 1;
        }
    }

    buffer[pos] = '\0';
    terminal_write(buffer);
}

static void kpanic(const char* msg) {
    terminal_write(msg);
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void init_kernel_memory(uint32_t* kernel_end)
{
    last_alloc = (uint32_t)kernel_end + 0x1000;
    heap_begin = last_alloc;
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end = pheap_begin;

    memset((char*)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t*)malloc(MAX_PAGE_ALIGNED_ALLOCS);

    terminal_write("Kernel heap starts at ");
    kprint_hex(last_alloc);
    terminal_write("\n");
}

void print_memory_layout(void)
{
    terminal_write("Memory used: ");
    kprint_dec(memory_used);
    terminal_write(" bytes\n");

    terminal_write("Memory free: ");
    kprint_dec(heap_end - heap_begin - memory_used);
    terminal_write(" bytes\n");

    terminal_write("Heap size: ");
    kprint_dec(heap_end - heap_begin);
    terminal_write(" bytes\n");

    terminal_write("Heap start: ");
    kprint_hex(heap_begin);
    terminal_write("\n");

    terminal_write("Heap end: ");
    kprint_hex(heap_end);
    terminal_write("\n");

    terminal_write("PHeap start: ");
    kprint_hex(pheap_begin);
    terminal_write("\n");

    terminal_write("PHeap end: ");
    kprint_hex(pheap_end);
    terminal_write("\n");
}

void free(void* mem)
{
    alloc_t* alloc = (alloc_t*)((uint8_t*)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

void pfree(void* mem)
{
    uint32_t mem_addr = (uint32_t)mem;

    if (mem_addr < pheap_begin || mem_addr > pheap_end) {
        return;
    }

    uint32_t ad = mem_addr;
    ad -= pheap_begin;
    ad /= 4096;

    pheap_desc[ad] = 0;
}

char* pmalloc(size_t size)
{
    (void)size;

    for (int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (pheap_desc[i]) {
            continue;
        }

        pheap_desc[i] = 1;

        terminal_write("PAllocated from ");
        kprint_hex(pheap_begin + i * 4096);
        terminal_write(" to ");
        kprint_hex(pheap_begin + (i + 1) * 4096);
        terminal_write("\n");

        return (char*)(pheap_begin + i * 4096);
    }

    terminal_write("pmalloc: FATAL: failure!\n");
    return 0;
}

void* malloc(size_t size)
{
    if (!size) {
        return 0;
    }

    uint8_t* mem = (uint8_t*)heap_begin;

    while ((uint32_t)mem < last_alloc) {
        alloc_t* a = (alloc_t*)mem;

        if (!a->size) {
            goto nalloc;
        }

        if (a->status) {
            mem += a->size;
            mem += sizeof(alloc_t);
            mem += 4;
            continue;
        }

        if (a->size >= size) {
            a->status = 1;
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);

            terminal_write("RE:Allocated ");
            kprint_dec((uint32_t)size);
            terminal_write(" bytes from ");
            kprint_hex((uint32_t)(mem + sizeof(alloc_t)));
            terminal_write(" to ");
            kprint_hex((uint32_t)(mem + sizeof(alloc_t) + size));
            terminal_write("\n");

            return (void*)(mem + sizeof(alloc_t));
        }

        mem += a->size;
        mem += sizeof(alloc_t);
        mem += 4;
    }

nalloc:
    if (last_alloc + size + sizeof(alloc_t) >= heap_end) {
        kpanic("Cannot allocate bytes! Out of memory.\n");
    }

    alloc_t* alloc = (alloc_t*)last_alloc;
    alloc->status = 1;
    alloc->size = (uint32_t)size;

    last_alloc += (uint32_t)size;
    last_alloc += sizeof(alloc_t);
    last_alloc += 4;

    memory_used += (uint32_t)size + 4 + sizeof(alloc_t);
    memset((char*)((uint32_t)alloc + sizeof(alloc_t)), 0, size);

    terminal_write("Allocated ");
    kprint_dec((uint32_t)size);
    terminal_write(" bytes from ");
    kprint_hex((uint32_t)alloc + sizeof(alloc_t));
    terminal_write(" to ");
    kprint_hex(last_alloc);
    terminal_write("\n");

    return (void*)((uint32_t)alloc + sizeof(alloc_t));
}
