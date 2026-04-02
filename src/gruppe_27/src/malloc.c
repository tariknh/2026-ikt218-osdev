#include "memory.h"
#include "terminal.h"
 
// ─── Constants ───────────────────────────────────────────────────────────────
 
#define PAGE_SIZE        4096u
#define HEAP_INITIAL    (PAGE_SIZE * 16)    // 64 KiB committed up front
#define HEAP_MAX        (PAGE_SIZE * 1024)  // 4 MiB absolute ceiling
#define SPLIT_THRESHOLD  16                 // don't split if leftover < this
 
// ─── Helpers ─────────────────────────────────────────────────────────────────
 
static inline uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + align - 1) & ~(align - 1);
}
 
static void print_hex(uint32_t v) {
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 9; i >= 2; i--) {
        uint32_t n = v & 0xF;
        buf[i] = (n < 10) ? ('0' + n) : ('A' + n - 10);
        v >>= 4;
    }
    buf[10] = '\0';
    terminal_write(buf);
}
 
static void print_dec(uint32_t v) {
    if (v == 0) { terminal_write("0"); return; }
    char buf[11]; int i = 10;
    buf[i] = '\0';
    while (v) { buf[--i] = '0' + (v % 10); v /= 10; }
    terminal_write(buf + i);
}
 
// ─── Block header ────────────────────────────────────────────────────────────
 
typedef struct block_header {
    size_t              size;   // bytes of user payload
    int                 free;   // 1 = available
    struct block_header *next;  // next block in list (NULL = last)
} block_header_t;
 
// ─── Heap state ──────────────────────────────────────────────────────────────
 
static uint8_t        *heap_start = 0;
static uint8_t        *heap_end   = 0;  // one past the last committed byte
static uint8_t        *heap_max   = 0;  // absolute ceiling
static block_header_t *free_list  = 0;  // head of the block list
 
// ─── extend_heap — commit more pages ─────────────────────────────────────────
 
static int extend_heap(size_t bytes) {
    bytes = align_up((uint32_t)bytes, PAGE_SIZE);
    if (heap_end + bytes > heap_max) {
        terminal_write("[kmalloc] heap exhausted!\n");
        return -1;
    }
    heap_end += bytes;
    return 0;
}
 
// ─── init_kernel_memory ──────────────────────────────────────────────────────
 
void init_kernel_memory(uint32_t *kernel_end) {
    heap_start = (uint8_t *)align_up((uint32_t)kernel_end, PAGE_SIZE);
    heap_end   = heap_start;
    heap_max   = heap_start + HEAP_MAX;
 
    if (extend_heap(HEAP_INITIAL) != 0) {
        terminal_write("[kmalloc] FATAL: cannot initialise heap\n");
        return;
    }
 
    // One giant free block covering the whole initial region.
    free_list        = (block_header_t *)heap_start;
    free_list->size  = HEAP_INITIAL - sizeof(block_header_t);
    free_list->free  = 1;
    free_list->next  = 0;
 
    terminal_write("[kmalloc] heap at ");
    print_hex((uint32_t)heap_start);
    terminal_write(", size: ");
    print_dec(HEAP_INITIAL / 1024);
    terminal_write(" KiB\n");
}
 
// ─── coalesce — merge adjacent free blocks ─────────────────────────────────
 
static void coalesce() {
    block_header_t *cur = free_list;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += sizeof(block_header_t) + cur->next->size;
            cur->next  = cur->next->next;
            // Don't advance — re-check in case three blocks are all free.
        } else {
            cur = cur->next;
        }
    }
}
 
// ─── malloc ──────────────────────────────────────────────────────────────────
 
void *malloc(size_t size) {
    if (size == 0) return 0;
 
    // 4-byte alignment so structs never straddle boundaries.
    size = align_up((uint32_t)size, 4);
 
    // First-fit search.
    block_header_t *cur = free_list;
    while (cur) {
        if (cur->free && cur->size >= size) {
            // Split if the leftover is worth keeping.
            size_t leftover = cur->size - size;
            if (leftover > sizeof(block_header_t) + SPLIT_THRESHOLD) {
                block_header_t *new_blk =
                    (block_header_t *)((uint8_t *)(cur + 1) + size);
                new_blk->size = leftover - sizeof(block_header_t);
                new_blk->free = 1;
                new_blk->next = cur->next;
                cur->next     = new_blk;
                cur->size     = size;
            }
            cur->free = 0;
            return (void *)(cur + 1);
        }
        cur = cur->next;
    }
 
    // No suitable block — grow the heap and retry (at most once).
    if (extend_heap(sizeof(block_header_t) + size) != 0) return 0;
    coalesce();
    return malloc(size);
}
    void* memmove(void* dest, const void* src, size_t n) {
        unsigned char* d = dest;
        const unsigned char* s = src;
        if (d < s) {
            while (n--) *d++ = *s++;
        } else {
            d += n;
            s += n;
            while (n--) *--d = *--s;
        }
        return dest;
    }
 
// ─── free ────────────────────────────────────────────────────────────────────
 
void free(void *ptr) {
    if (!ptr) return;
    block_header_t *hdr = (block_header_t *)ptr - 1;
    hdr->free = 1;
    coalesce();
}