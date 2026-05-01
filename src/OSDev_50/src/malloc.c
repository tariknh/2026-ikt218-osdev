#include "memory.h"
#include <libc/system.h>


#define MALLOC_DEBUG 0

#if MALLOC_DEBUG
  #define DBG(...) printf(__VA_ARGS__)
#else
  #define DBG(...) ((void)0)
#endif

#define MAX_PAGE_ALIGNED_ALLOCS 32

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t *pheap_desc = 0;
uint32_t memory_used = 0;

void init_kernel_memory(uint32_t* kernel_end)
{
    last_alloc = (uint32_t)kernel_end + 0x1000;

    heap_begin = last_alloc;

    pheap_end   = 0x400000; 
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end    = pheap_begin;

    memset((char*)heap_begin, 0, heap_end - heap_begin);

    pheap_desc = (uint8_t*)malloc(MAX_PAGE_ALIGNED_ALLOCS);
    memset((char*)pheap_desc, 0, MAX_PAGE_ALIGNED_ALLOCS);

    printf("Kernel heap starts at 0x%x\n", last_alloc);
}

void print_memory_layout()
{
    printf("Memory used: %d bytes\n", memory_used);
    printf("Memory free: %d bytes\n", heap_end - heap_begin - memory_used);
    printf("Heap size: %d bytes\n", heap_end - heap_begin);
    printf("Heap start: 0x%x\n", heap_begin);
    printf("Heap end: 0x%x\n", heap_end);
    printf("PHeap start: 0x%x\nPHeap end: 0x%x\n", pheap_begin, pheap_end);
}

void free(void *mem)
{
    if(!mem) return;

    alloc_t *alloc = (alloc_t *)((uint8_t*)mem - sizeof(alloc_t));

    memory_used -= (alloc->size + sizeof(alloc_t) + 4);
    alloc->status = 0;
}

void pfree(void *mem)
{
    if(mem < (void*)pheap_begin || mem > (void*)pheap_end) return;

    uint32_t ad = (uint32_t)mem;
    ad -= pheap_begin;
    ad /= 4096;

    pheap_desc[ad] = 0;
}

char* pmalloc(size_t size)
{
    (void)size;

    for(int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++)
    {
        if(pheap_desc[i]) continue;

        pheap_desc[i] = 1;
        DBG("PAllocated from 0x%x to 0x%x\n",
            pheap_begin + i*4096, pheap_begin + (i+1)*4096);

        return (char *)(pheap_begin + i*4096);
    }

    panic("pmalloc: Out of page-aligned memory.\n");
    return 0;
}

void* malloc(size_t size)
{
    if(!size) return 0;

    uint8_t *mem = (uint8_t *)heap_begin;

    while((uint32_t)mem < last_alloc)
    {
        alloc_t *a = (alloc_t *)mem;

        DBG("mem=0x%x a={.status=%d, .size=%d}\n", mem, a->status, a->size);

        if(!a->size)
            goto nalloc;

        if(a->status) {
            mem += a->size + sizeof(alloc_t) + 4;
            continue;
        }

        if(a->size >= size)
        {
            a->status = 1;
            a->size = size; 

            DBG("RE:Allocated %d bytes from 0x%x to 0x%x\n",
                (int)size,
                (uint32_t)(mem + sizeof(alloc_t)),
                (uint32_t)(mem + sizeof(alloc_t) + size));

            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += (size + sizeof(alloc_t) + 4);

            return (char *)(mem + sizeof(alloc_t));
        }

        mem += a->size + sizeof(alloc_t) + 4;
    }

nalloc:
    if(last_alloc + size + sizeof(alloc_t) + 4 >= heap_end)
        panic("Cannot allocate bytes! Out of memory.\n");

    alloc_t *alloc = (alloc_t *)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    last_alloc += size + sizeof(alloc_t) + 4;
    memory_used += (size + sizeof(alloc_t) + 4);

    DBG("Allocated %d bytes from 0x%x to 0x%x\n",
        (int)size,
        (uint32_t)alloc + sizeof(alloc_t),
        last_alloc);

    memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char *)((uint32_t)alloc + sizeof(alloc_t));
}