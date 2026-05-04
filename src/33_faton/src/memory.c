#include <memory.h>
#include <monitor.h>
#include <libc/stdint.h>
#include <libc/stddef.h>

#define MAX_PAGE_ALIGNED_ALLOCS 32

static uint32_t last_alloc  = 0;
static uint32_t heap_end    = 0;
static uint32_t heap_begin  = 0;
static uint32_t pheap_begin = 0;
static uint32_t pheap_end   = 0;
static uint8_t* pheap_desc  = 0;
static uint32_t memory_used = 0;

void init_kernel_memory(uint32_t* kernel_end) {
    last_alloc  = (uint32_t)kernel_end + 0x1000;
    heap_begin  = last_alloc;
    pheap_end   = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end    = pheap_begin;
    memset((char*)heap_begin, 0, heap_end - heap_begin);
    pheap_desc  = (uint8_t*)malloc(MAX_PAGE_ALIGNED_ALLOCS);
    printf("Kernel heap starts at 0x%x\n", last_alloc);
}

void print_memory_layout(void) {
    printf("Memory used: %d bytes\n", memory_used);
    printf("Memory free: %d bytes\n", heap_end - heap_begin - memory_used);
    printf("Heap size:   %d bytes\n", heap_end - heap_begin);
    printf("Heap start:  0x%x\n", heap_begin);
    printf("Heap end:    0x%x\n", heap_end);
}

void free(void* mem) {
    alloc_t* alloc = (alloc_t*)((uint32_t)mem - sizeof(alloc_t));
    memory_used   -= alloc->size + sizeof(alloc_t);
    alloc->status  = 0;
}

void* malloc(size_t size) {
    if (!size) return 0;

    uint8_t* mem = (uint8_t*)heap_begin;
    while ((uint32_t)mem < last_alloc) {
        alloc_t* a = (alloc_t*)mem;
        if (!a->size)
            goto nalloc;
        if (a->status) {
            mem += a->size + sizeof(alloc_t) + 4;
            continue;
        }
        if (a->size >= size) {
            a->status = 1;
            memset((char*)mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (char*)(mem + sizeof(alloc_t));
        }
        mem += a->size + sizeof(alloc_t) + 4;
    }

nalloc:
    if (last_alloc + size + sizeof(alloc_t) >= heap_end) {
        printf("malloc: out of memory!\n");
        return 0;
    }
    alloc_t* alloc  = (alloc_t*)last_alloc;
    alloc->status   = 1;
    alloc->size     = size;
    last_alloc     += size + sizeof(alloc_t) + 4;
    memory_used    += size + sizeof(alloc_t);
    memset((char*)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char*)((uint32_t)alloc + sizeof(alloc_t));
}

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = ptr;
    while (num--)
        *p++ = (unsigned char)value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t count) {
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;
    while (count--)
        *dst8++ = *src8++;
    return dest;
}