#include <memory.h>
#include <libc/stdint.h>
#include <libc/stddef.h>
#include <libc/stdio.h>

/* ─── Forward declarations ─── */
extern void terminal_write(const char *str);

/* ─── Panic: halt the kernel on fatal error ─── */
void panic(const char *msg)
{
    printf("PANIC: %s", msg);
    while (1)
        __asm__ volatile("hlt");
}

/* ═══════════════════════════════════════════════════════
 * Memory Utilities (memcpy, memset, memset16)
 * ═══════════════════════════════════════════════════════ */

void* memcpy(void* dest, const void* src, size_t count)
{
    char* dst8 = (char*)dest;
    char* src8 = (char*)src;

    if (count & 1) {
        dst8[0] = src8[0];
        dst8 += 1;
        src8 += 1;
    }

    count /= 2;
    while (count--) {
        dst8[0] = src8[0];
        dst8[1] = src8[1];
        dst8 += 2;
        src8 += 2;
    }

    return dest;
}

void* memset16(void *ptr, uint16_t value, size_t num)
{
    uint16_t* p = ptr;
    while (num--)
        *p++ = value;
    return ptr;
}

void* memset(void *ptr, int value, size_t num)
{
    unsigned char* p = ptr;
    while (num--)
        *p++ = (unsigned char)value;
    return ptr;
}

/* ═══════════════════════════════════════════════════════
 * Heap Allocator (malloc / free / pmalloc / pfree)
 * ═══════════════════════════════════════════════════════ */

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
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096);
    heap_end = pheap_begin;
    memset((char *)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS);
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
    alloc_t *alloc = (alloc_t *)((uint8_t *)mem - sizeof(alloc_t));
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
        printf("PAllocated from 0x%x to 0x%x\n", pheap_begin + i*4096, pheap_begin + (i+1)*4096);
        return (char *)(pheap_begin + i*4096);
    }
    printf("pmalloc: FATAL: failure!\n");
    return 0;
}

void* malloc(size_t size)
{
    if (!size) return 0;

    /* Walk existing blocks looking for a freed one that fits */
    uint8_t *mem = (uint8_t *)heap_begin;
    while ((uint32_t)mem < last_alloc)
    {
        alloc_t *a = (alloc_t *)mem;

        if (!a->size)
            goto nalloc;
        if (a->status) {
            mem += a->size;
            mem += sizeof(alloc_t);
            mem += 4;
            continue;
        }
        if (a->size >= size)
        {
            a->status = 1;
            printf("RE:Allocated %d bytes from 0x%x to 0x%x\n", size,
                   (uint32_t)(mem + sizeof(alloc_t)),
                   (uint32_t)(mem + sizeof(alloc_t) + size));
            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (char *)(mem + sizeof(alloc_t));
        }
        mem += a->size;
        mem += sizeof(alloc_t);
        mem += 4;
    }

nalloc:;
    if (last_alloc + size + sizeof(alloc_t) >= heap_end)
    {
        panic("Cannot allocate bytes! Out of memory.\n");
    }
    alloc_t *alloc = (alloc_t *)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    last_alloc += size;
    last_alloc += sizeof(alloc_t);
    last_alloc += 4;
    printf("Allocated %d bytes from 0x%x to 0x%x\n", size,
           (uint32_t)alloc + sizeof(alloc_t), last_alloc);
    memory_used += size + 4 + sizeof(alloc_t);
    memset((char *)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char *)((uint32_t)alloc + sizeof(alloc_t));
}

/* ═══════════════════════════════════════════════════════
 * Paging
 * ═══════════════════════════════════════════════════════ */

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint16_t id = virt >> 22;
    for (int i = 0; i < 1024; i++)
    {
        last_page[i] = phys | 3;
        phys += 4096;
    }
    page_directory[id] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t *)(((uint32_t)last_page) + 4096);
}

static void paging_enable()
{
    __asm__ volatile("mov %%eax, %%cr3": :"a"(page_dir_loc));
    __asm__ volatile("mov %cr0, %eax");
    __asm__ volatile("orl $0x80000000, %eax");
    __asm__ volatile("mov %eax, %cr0");
}

void init_paging()
{
    printf("Setting up paging\n");
    page_directory = (uint32_t*)0x400000;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t *)0x404000;
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0 | 2;
    }
    paging_map_virtual_to_phys(0, 0);
    paging_map_virtual_to_phys(0x400000, 0x400000);
    paging_enable();
    printf("Paging was successfully enabled!\n");
}
