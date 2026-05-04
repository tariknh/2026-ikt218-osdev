#include "kernel/memory.h"

#include "stdio.h"

#define MAX_PAGE_ALIGNED_ALLOCS 32
#define PAGE_SIZE 4096
#define ALLOC_FREE 0
#define ALLOC_USED 1

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t *pheap_desc = 0;
uint32_t memory_used = 0;

void init_kernel_memory(uint32_t *kernel_end) {
    last_alloc = (uint32_t) kernel_end + 0x1000;
    heap_begin = last_alloc;

    // Keep the top of the 1-4 MB heap range for page-aligned allocations
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * PAGE_SIZE);
    heap_end = pheap_begin;
    memory_used = 0;

    memset((void *) heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t *) malloc(MAX_PAGE_ALIGNED_ALLOCS);
    printf("Kernel heap starts at 0x%x\n", last_alloc);
}

void print_memory_layout() {
    printf("Memory used: %d bytes\n", memory_used);
    printf("Memory free: %d bytes\n", heap_end - heap_begin - memory_used);
    printf("Heap size: %d bytes\n", heap_end - heap_begin);
    printf("Heap start: 0x%x\n", heap_begin);
    printf("Heap end: 0x%x\n", heap_end);
    printf("PHeap start: 0x%x\n", pheap_begin);
    printf("PHeap end: 0x%x\n", pheap_end);
}

char *pmalloc(size_t size) {
    (void) size;

    if (pheap_desc == 0) {
        return 0;
    }

    for (int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (pheap_desc[i] != 0) {
            continue;
        }

        pheap_desc[i] = 1;
        printf("PAllocated from 0x%x to 0x%x\n", pheap_begin + i * PAGE_SIZE, pheap_begin + (i + 1) * PAGE_SIZE);
        return (char *) (pheap_begin + i * PAGE_SIZE);
    }

    printf("pmalloc: failed\n");
    return 0;
}

void pfree(void *mem) {
    if (pheap_desc == 0 || (uint32_t) mem < pheap_begin || (uint32_t) mem >= pheap_end) {
        return;
    }

    uint32_t page_id = ((uint32_t) mem - pheap_begin) / PAGE_SIZE;
    pheap_desc[page_id] = 0;
}

void *malloc(size_t size) {
    if (size == 0) {
        return 0;
    }

    // Normal heap blocks are stored as an alloc_t header followed by the usable bytes
    uint8_t *mem = (uint8_t *) heap_begin;
    while ((uint32_t) mem < last_alloc) {
        alloc_t *alloc = (alloc_t *) mem;

        if (alloc->size == 0) {
            break;
        }

        if (alloc->status == ALLOC_USED) {
            mem += sizeof(alloc_t) + alloc->size + 4;
            continue;
        }

        if (alloc->size >= size) {
            alloc->status = ALLOC_USED;
            memory_used += alloc->size + sizeof(alloc_t) + 4;
            memset(mem + sizeof(alloc_t), 0, size);
            return mem + sizeof(alloc_t);
        }

        mem += sizeof(alloc_t) + alloc->size + 4;
    }

    if (last_alloc + sizeof(alloc_t) + size + 4 >= heap_end) {
        printf("malloc: out of memory\n");
        return 0;
    }

    alloc_t *alloc = (alloc_t *) last_alloc;
    alloc->status = ALLOC_USED;
    alloc->size = size;

    last_alloc += sizeof(alloc_t) + size + 4;
    memory_used += size + sizeof(alloc_t) + 4;

    memset((void *) ((uint32_t) alloc + sizeof(alloc_t)), 0, size);

    return (void *) ((uint32_t) alloc + sizeof(alloc_t));
}

void free(void *mem) {
    if (mem == 0) {
        return;
    }

    alloc_t *block = (alloc_t *) ((uint32_t) mem - sizeof(alloc_t));
    if (block->status == ALLOC_FREE) {
        return;
    }

    block->status = ALLOC_FREE;
    memory_used -= block->size + sizeof(alloc_t) + 4;
}
