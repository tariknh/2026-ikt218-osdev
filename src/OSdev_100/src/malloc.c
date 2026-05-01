#include "memory.h"

extern void printf(const char* format, ...);
extern void panic(const char* message);
#define MAX_PAGE_ALIGNED_ALLOCS 32

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t *pheap_desc = 0;
uint32_t memory_used = 0;

// split free memory into a normal heap and a small page-aligned heap
void init_kernel_memory(uint32_t* kernel_end)
{
    last_alloc = (uint32_t)kernel_end + 0x1000;   // start just after the kernel
    heap_begin = last_alloc;                      // normal heap starts here
    pheap_end = 0x400000;                         // keep top of low memory for page-aligned blocks
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end = pheap_begin;                       // normal heap stops where pheap begins
    memset((char *)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS); // one byte per page-aligned slot so we know which ones are used
    printf("Kernel heap starts at 0x%x\n", last_alloc);
}

// Print the current memory layout
void print_memory_layout()
{
    printf("Memory used: %d bytes\n", memory_used);
    printf("Memory free: %d bytes\n", heap_end - heap_begin - memory_used);
    printf("Heap size: %d bytes\n", heap_end - heap_begin);
    printf("Heap start: 0x%x\n", heap_begin);
    printf("Heap end: 0x%x\n", heap_end);
    printf("PHeap start: 0x%x\nPHeap end: 0x%x\n", pheap_begin, pheap_end);
}

// Free a block of memory
void free(void *mem)
{
    alloc_t *alloc = (alloc_t *)((char *)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

// free one page-aligned slot so it can be reused later
void pfree(void *mem)
{
    uint32_t ad = (uint32_t)mem; 
    if(ad < pheap_begin || ad > pheap_end) return;

    ad -= pheap_begin;
    ad /= 4096;

    pheap_desc[ad] = 0;
}


// give back one free 4 KB aligned block
char* pmalloc(size_t size)
{
    for(int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++)
    {
        if(pheap_desc[i]) continue;
        pheap_desc[i] = 1;
        printf("PAllocated from 0x%x to 0x%x\n", pheap_begin + i*4096, pheap_begin + (i+1)*4096);
        return (char *)(pheap_begin + i*4096);
    }
    printf("pmalloc: FATAL: failure!\n");
    return 0;
}


// simple heap allocator: reuse an old block if possible, otherwise take fresh space from last_alloc
void* malloc(size_t size)
{
    if(!size) return 0;

    uint8_t *mem = (uint8_t *)heap_begin;
        while((uint32_t)mem < last_alloc)
        {
            alloc_t *a = (alloc_t *)mem;
            
            if(!a->size) goto nalloc;
            
            if(a->status) {
                uint32_t next_addr = (uint32_t)mem + a->size + sizeof(alloc_t) + 4;
                mem = (uint8_t *)next_addr; 
                continue;
            }
        // if this old block is free and big enough, just reuse it
        if(a->size >= size)
        {
            a->status = 1;
            printf("RE:Allocated %d bytes from 0x%x to 0x%x\n", size, mem + sizeof(alloc_t), mem + sizeof(alloc_t) + size);
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (char *)(mem + sizeof(alloc_t));
        }
        // otherwise skip past this block and keep looking
        mem += a->size;
        mem += sizeof(alloc_t);
        mem += 4;
    }

    nalloc:;
    // no old block worked, so make a fresh one from the end of the heap
    if(last_alloc + size + sizeof(alloc_t) >= heap_end)
    {
        panic("Cannot allocate bytes! Out of memory.\n");
    }
    alloc_t *alloc = (alloc_t *)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    last_alloc += size;
    last_alloc += sizeof(alloc_t);
    last_alloc += 4;
    printf("Allocated %d bytes from 0x%x to 0x%x\n", size, (uint32_t)alloc + sizeof(alloc_t), last_alloc);
    memory_used += size + 4 + sizeof(alloc_t);
    memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char *)((uint32_t)alloc + sizeof(alloc_t));
}
