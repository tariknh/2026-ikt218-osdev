/*
 * memory.c - Kernel heap allocator (linked-list / first-fit)
 *
 * Heap layout
 * -----------
 *   [block_meta][user data][block_meta][user data] ...
 *
 * Each block is preceded by a metadata header (size, free-flag, next-pointer).
 * malloc() walks the list looking for a free block big enough; if it finds
 * one it marks it used and returns the user-data pointer.  If it doesn't,
 * the heap is extended by carving a new block out of the unused area.
 *
 * The allocator uses a fixed-size heap (HEAP_SIZE bytes) located right after
 * the kernel image.  That region is well within the 4 MiB identity-mapped
 * by paging, so it remains addressable both before and after init_paging().
 */

#include <memory.h>
#include <libc/stdint.h>
#include <libc/stddef.h>
#include <libc/stdio.h>

#define HEAP_SIZE   (1u * 1024u * 1024u)   /* 1 MiB */
#define ALIGN       4

typedef struct block_meta {
    size_t              size;       /* size of the user data area */
    int                 free;       /* 1 = available, 0 = in use */
    struct block_meta*  next;
} block_meta_t;

#define META_SIZE   (sizeof(block_meta_t))

static block_meta_t*   heap_head      = (void*)0;
static uint32_t        heap_break;     /* next unused byte in the heap */
static uint32_t        heap_start_addr;
static uint32_t        heap_end_addr;

static size_t align_up(size_t v) {
    return (v + (ALIGN - 1)) & ~(size_t)(ALIGN - 1);
}

static block_meta_t* extend_heap(size_t size) {
    if (heap_break + META_SIZE + size > heap_end_addr) {
        return (void*)0;
    }
    block_meta_t* block = (block_meta_t*)heap_break;
    block->size = size;
    block->free = 0;
    block->next = (void*)0;
    heap_break += META_SIZE + size;
    return block;
}

static block_meta_t* find_free(size_t size) {
    block_meta_t* curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            return curr;
        }
        curr = curr->next;
    }
    return (void*)0;
}

void init_kernel_memory(uint32_t* kernel_end) {
    heap_start_addr = (uint32_t)kernel_end;
    /* round up to a page boundary so the heap starts cleanly */
    heap_start_addr = (heap_start_addr + 0xFFF) & ~0xFFFu;
    heap_break      = heap_start_addr;
    heap_end_addr   = heap_start_addr + HEAP_SIZE;
    heap_head       = (void*)0;

    printf("[mem] kernel ends at 0x%x, heap = 0x%x .. 0x%x\n",
           (unsigned int)kernel_end,
           (unsigned int)heap_start_addr,
           (unsigned int)heap_end_addr);
}

void* malloc(size_t size) {
    if (size == 0) return (void*)0;
    size = align_up(size);

    block_meta_t* block;

    if (heap_head == (void*)0) {
        block = extend_heap(size);
        if (!block) return (void*)0;
        heap_head = block;
    } else {
        block = find_free(size);
        if (block) {
            block->free = 0;
        } else {
            block_meta_t* tail = heap_head;
            while (tail->next) tail = tail->next;
            block = extend_heap(size);
            if (!block) return (void*)0;
            tail->next = block;
        }
    }

    return (void*)((uint32_t)block + META_SIZE);
}

void free(void* ptr) {
    if (!ptr) return;
    block_meta_t* block = (block_meta_t*)((uint32_t)ptr - META_SIZE);
    block->free = 1;
}

void print_memory_layout(void) {
    printf("[mem] heap layout:\n");
    printf("      start  = 0x%x\n", (unsigned int)heap_start_addr);
    printf("      break  = 0x%x  (used = %d bytes)\n",
           (unsigned int)heap_break,
           (int)(heap_break - heap_start_addr));
    printf("      end    = 0x%x\n", (unsigned int)heap_end_addr);

    int n = 0, used = 0, freec = 0;
    for (block_meta_t* b = heap_head; b; b = b->next) {
        n++;
        if (b->free) freec++; else used++;
    }
    printf("      blocks = %d total, %d used, %d free\n", n, used, freec);
}
