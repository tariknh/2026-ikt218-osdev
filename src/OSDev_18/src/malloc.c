#include <kernel/memory.h>
#include <kernel/terminal.h>
#include <libc/stdint.h>

#define MAX_PAGE_ALIGNED_ALLOCS 32

static uint32_t last_alloc = 0;
static uint32_t heap_end = 0;
static uint32_t heap_begin = 0;
static uint32_t pheap_begin = 0;
static uint32_t pheap_end = 0;
static uint8_t *pheap_desc = 0;
static uint32_t memory_used = 0;

void InitKernelMemory(uint32_t* kernel_end) {
    uint32_t kernelEndAddr = (uint32_t)kernel_end;

    // Leave one page after the kernel image before starting the heap.
    last_alloc = kernelEndAddr + 0x1000;
    heap_begin = last_alloc;
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end = pheap_begin;
    memset((char*)heap_begin, 0, heap_end - heap_begin);
    // Reuse the normal heap to track which page-sized slots are in use.
    pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS);

    TerminalWriteString("Kernel heap starts at 0x");
    TerminalWriteUInt(last_alloc);
    TerminalWriteString("\n");
}

void PrintMemoryLayout(void) {
    TerminalWriteString("Memory used:");
    TerminalWriteUInt(memory_used);
    TerminalWriteString(" bytes\n");

    TerminalWriteString("Memory free:");
    TerminalWriteUInt(heap_end - heap_begin - memory_used);
    TerminalWriteString(" bytes\n");

    TerminalWriteString("Heap size:");
    TerminalWriteUInt(heap_end - heap_begin);
    TerminalWriteString(" bytes\n");

    TerminalWriteString("Heap start: ");
    TerminalWriteHex(heap_begin);
    TerminalWriteString("\n");

    TerminalWriteString("Heap end: ");
    TerminalWriteHex(heap_end);
    TerminalWriteString("\n");

    TerminalWriteString("PHeap start: ");
    TerminalWriteHex(pheap_begin);
    TerminalWriteString("\n");

    TerminalWriteString("PHeap end: ");
    TerminalWriteHex(pheap_end);
    TerminalWriteString("\n");
}

void free(void *mem) {
    if (!mem) return;

    alloc_t* alloc = (alloc_t*)((uint8_t*)mem - sizeof(alloc_t));

    if (alloc->status) {
        memory_used -= alloc->size + sizeof(alloc_t) + 4;
        alloc->status = 0;
    }
}

void pfree(void *mem) {
    uint32_t addr = (uint32_t)mem;

    if (addr < pheap_begin || addr >= pheap_end) return;

    // Convert the address back into the descriptor index for that 4 KiB slot.
    addr -= pheap_begin;
    addr /= 4096;
    pheap_desc[addr] = 0;
}

char* pmalloc(size_t size) {
    (void) size;

    for (int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (pheap_desc[i]) continue;

        pheap_desc[i] = 1;
        TerminalWriteString("PAllocated from 0x");
        TerminalWriteUInt(pheap_begin + i * 4096);
        TerminalWriteString(" to 0x");
        TerminalWriteUInt(pheap_begin + (i+1) * 4096);
        TerminalWriteString("\n");

        return (char*)(pheap_begin + i * 4096);
    }
    TerminalWriteString("pmalloc: FATAL: failure!\n");
    return 0;
}

void* malloc(size_t size) {
    if(!size) return 0;

    uint8_t* mem = (uint8_t*)heap_begin;

    while ((uint32_t)mem < last_alloc) {
        alloc_t* a = (alloc_t*)mem;

        if(!a->size) goto nalloc;

        if(a->status) {
            mem += sizeof(alloc_t) + a->size + 4;
            continue;
        }
        
        if(a->size >= size) {
            a->status = 1;
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += a->size + sizeof(alloc_t) + 4;

            TerminalWriteString("Re-allocated ");
            TerminalWriteUInt(size);
            TerminalWriteString(" bytes from 0x");
            TerminalWriteUInt((uint32_t)(mem + sizeof(alloc_t)));
            TerminalWriteString(" to 0x");
            TerminalWriteUInt((uint32_t)(mem + sizeof(alloc_t) + size));
            TerminalWriteString("\n");

            return (char*)(mem + sizeof(alloc_t));
        }

        mem += a->size + sizeof(alloc_t) + 4;
    }

    nalloc:;
    // Each block stores its metadata before the returned payload.
    if (last_alloc + size + sizeof(alloc_t) + 4 >= heap_end) {
        TerminalWriteString("Cannot allocate bytes! Out of memory.\n");
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    alloc_t* alloc = (alloc_t*)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    last_alloc += size + sizeof(alloc_t) + 4;
    memory_used += size + 4 + sizeof(alloc_t);

    memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);

    TerminalWriteString("Allocated ");
    TerminalWriteUInt(size);
    TerminalWriteString(" bytes from 0x");
    TerminalWriteUInt((uint32_t)alloc + sizeof(alloc_t));
    TerminalWriteString(" to 0x");
    TerminalWriteUInt(last_alloc);
    TerminalWriteString("\n");

    return (char*)((uint32_t)alloc + sizeof(alloc_t));
}
