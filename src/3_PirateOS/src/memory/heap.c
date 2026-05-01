#include "memory/heap.h"
#include "libc/memory.h"
#include "libc/stdio.h"

/*
 * This file handles dynamic memory for the kernel
 * It has one normal heap for malloc/free and one small page-aligned area for pmalloc/pfree
 */

#define MAX_PAGE_ALIGNED_ALLOCS 32
#define PAGE_SIZE 4096
#define PAGE_REGION_END 0x400000
#define PAGE_CONTINUATION 0xFF
#define ALIGN4(value) (((value) + 3) & ~3U)

static uint32_t last_alloc = 0;
static uint32_t heap_end = 0;
static uint32_t heap_begin = 0;
static uint32_t pheap_begin = 0;
static uint32_t pheap_end = 0;
static uint8_t *pheap_desc = 0;
static uint32_t memory_used = 0;

static uint8_t *block_payload(alloc_t *block)
{
    // The usable memory comes right after the block header
    return ((uint8_t *)block) + sizeof(alloc_t);
}

static void merge_free_blocks(void)
{
    // Walk through the heap and join neighboring free blocks into larger free blocks
    uint8_t *cursor = (uint8_t *)heap_begin;

    while ((uint32_t)cursor < last_alloc) {
        alloc_t *block = (alloc_t *)cursor;
        uint8_t *next_ptr;
        alloc_t *next;

        if (block->size == 0) {
            break;
        }

        next_ptr = cursor + sizeof(alloc_t) + block->size;
        if ((uint32_t)next_ptr >= last_alloc) {
            break;
        }

        next = (alloc_t *)next_ptr;
        if (!block->status && !next->status && next->size != 0) {
            block->size += sizeof(alloc_t) + next->size;
            continue;
        }

        cursor = next_ptr;
    }
}

void init_kernel_memory(uint32_t *kernel_end)
{
    uint32_t kernel_addr = (uint32_t)kernel_end;

    // Start the heap on the next page after the kernel image
    kernel_addr = (kernel_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    last_alloc = kernel_addr + PAGE_SIZE;
    heap_begin = last_alloc;
    pheap_end = PAGE_REGION_END;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * PAGE_SIZE);
    heap_end = pheap_begin;

    // Clear the heap area so unused headers start as zero
    memset((void *)heap_begin, 0, heap_end - heap_begin);

    // The page allocator keeps one small descriptor byte per page slot
    pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS);
    if (pheap_desc != 0) {
        memset(pheap_desc, 0, MAX_PAGE_ALIGNED_ALLOCS);
    }
}

void print_memory_layout(void)
{
    printf("Memory used: %d bytes\n", (int)memory_used);
    printf("Memory free: %d bytes\n", (int)(heap_end - heap_begin - memory_used));
    printf("Heap size: %d bytes\n", (int)(heap_end - heap_begin));
    printf("Heap start: %d\n", (int)heap_begin);
    printf("Heap end: %d\n", (int)heap_end);
    printf("Page heap start: %d\n", (int)pheap_begin);
    printf("Page heap end: %d\n", (int)pheap_end);
}

void free(void *mem)
{
    alloc_t *block;
    uint8_t *address;

    if (mem == 0) {
        return;
    }

    address = (uint8_t *)mem;
    if ((uint32_t)address < heap_begin + sizeof(alloc_t) || (uint32_t)address >= last_alloc) {
        return;
    }

    block = (alloc_t *)(address - sizeof(alloc_t));
    if (!block->status) {
        return;
    }

    block->status = 0;
    memory_used -= block->size + sizeof(alloc_t);
    merge_free_blocks();
}

void pfree(void *mem)
{
    uint32_t address;
    uint32_t page_index;
    uint8_t pages;
    uint8_t i;

    if (mem == 0) {
        return;
    }

    address = (uint32_t)mem;
    if (address < pheap_begin || address >= pheap_end) {
        return;
    }

    page_index = (address - pheap_begin) / PAGE_SIZE;
    pages = pheap_desc[page_index];
    if (pages == 0 || pages == PAGE_CONTINUATION) {
        return;
    }

    // Clear all descriptor bytes that belong to this page allocation
    for (i = 0; i < pages; i++) {
        pheap_desc[page_index + i] = 0;
    }
}

char *pmalloc(size_t size)
{
    uint32_t pages;
    uint32_t run_length = 0;
    uint32_t run_start = 0;
    uint32_t i;

    if (size == 0 || pheap_desc == 0) {
        return 0;
    }

    pages = ((uint32_t)size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages > MAX_PAGE_ALIGNED_ALLOCS) {
        return 0;
    }

    // Look for enough free page slots in a row
    for (i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (pheap_desc[i] == 0) {
            if (run_length == 0) {
                run_start = i;
            }

            run_length++;
            if (run_length == pages) {
                uint32_t j;

                // Store the allocation length in the first slot
                pheap_desc[run_start] = (uint8_t)pages;
                for (j = 1; j < pages; j++) {
                    pheap_desc[run_start + j] = PAGE_CONTINUATION;
                }

                return (char *)(pheap_begin + (run_start * PAGE_SIZE));
            }
        } else {
            run_length = 0;
        }
    }

    return 0;
}

void *malloc(size_t size)
{
    uint32_t aligned_size;
    uint8_t *cursor;

    if (size == 0) {
        return 0;
    }

    aligned_size = ALIGN4((uint32_t)size);
    cursor = (uint8_t *)heap_begin;

    // Search from the start of the heap and use the first free block that fits
    while ((uint32_t)cursor < last_alloc) {
        alloc_t *block = (alloc_t *)cursor;

        if (block->size == 0) {
            break;
        }

        if (!block->status && block->size >= aligned_size) {
            uint32_t remaining = block->size - aligned_size;

            // If the free block is larger than needed, split it into two blocks
            if (remaining > sizeof(alloc_t) + 4) {
                alloc_t *split_block = (alloc_t *)(cursor + sizeof(alloc_t) + aligned_size);

                split_block->status = 0;
                split_block->size = remaining - sizeof(alloc_t);
                block->size = aligned_size;
            }

            block->status = 1;
            memory_used += block->size + sizeof(alloc_t);
            memset(block_payload(block), 0, block->size);
            return block_payload(block);
        }

        cursor += sizeof(alloc_t) + block->size;
    }

    if (last_alloc + sizeof(alloc_t) + aligned_size > heap_end) {
        return 0;
    }

    // If no old free block fits, place the new block at the end of the heap
    cursor = (uint8_t *)last_alloc;
    ((alloc_t *)cursor)->status = 1;
    ((alloc_t *)cursor)->size = aligned_size;
    last_alloc += sizeof(alloc_t) + aligned_size;
    memory_used += aligned_size + sizeof(alloc_t);
    memset(block_payload((alloc_t *)cursor), 0, aligned_size);

    return block_payload((alloc_t *)cursor);
}