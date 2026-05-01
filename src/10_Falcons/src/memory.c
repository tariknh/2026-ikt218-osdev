#include <kernel/memory.h>
#include <kernel/terminal.h>

#define ALLOC_MAGIC 0xC0FFEE42u
#define HEAP_LIMIT  0x400000u      /* keep the simple heap below 4 MiB */
#define PAGE_SIZE   4096u
#define MAX_PAGE_ALIGNED_ALLOCS 64u

static alloc_header_t *heap_head = 0;
static uint32_t heap_start = 0;
static uint32_t heap_end = HEAP_LIMIT;
static uint32_t memory_used = 0;

static uint32_t pheap_start = 0;
static uint32_t pheap_end = 0;
static uint8_t pheap_bitmap[MAX_PAGE_ALIGNED_ALLOCS];

static uint32_t align_up(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void *ptr, int value, size_t count)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)value;
    }
    return ptr;
}

void *memset16(void *ptr, uint16_t value, size_t count)
{
    uint16_t *p = (uint16_t *)ptr;
    for (size_t i = 0; i < count; i++) {
        p[i] = value;
    }
    return ptr;
}

void init_kernel_memory(uint32_t *kernel_end)
{
    heap_start = align_up((uint32_t)kernel_end, 16u);

    /* Reserve the last part of the first 4 MiB for page-aligned allocations. */
    pheap_end = HEAP_LIMIT;
    pheap_start = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * PAGE_SIZE);
    heap_end = pheap_start;

    if (heap_start >= heap_end) {
        printf("[MEM] ERROR: kernel image is too large for the simple heap.\n");
        return;
    }

    heap_head = (alloc_header_t *)heap_start;
    heap_head->magic = ALLOC_MAGIC;
    heap_head->size = heap_end - heap_start - sizeof(alloc_header_t);
    heap_head->free = 1;
    heap_head->next = 0;

    memory_used = 0;
    memset(pheap_bitmap, 0, sizeof(pheap_bitmap));

    printf("[MEM] Kernel end: 0x%x\n", (uint32_t)kernel_end);
    printf("[MEM] Heap initialized: 0x%x - 0x%x\n", heap_start, heap_end);
}

static void split_block(alloc_header_t *block, uint32_t requested_size)
{
    uint32_t remaining = block->size - requested_size;
    if (remaining <= sizeof(alloc_header_t) + 16u) {
        return;
    }

    alloc_header_t *new_block = (alloc_header_t *)((uint8_t *)block + sizeof(alloc_header_t) + requested_size);
    new_block->magic = ALLOC_MAGIC;
    new_block->size = remaining - sizeof(alloc_header_t);
    new_block->free = 1;
    new_block->next = block->next;

    block->size = requested_size;
    block->next = new_block;
}

void *malloc(size_t size)
{
    if (size == 0 || heap_head == 0) {
        return 0;
    }

    uint32_t requested_size = align_up((uint32_t)size, 8u);
    alloc_header_t *current = heap_head;

    while (current) {
        if (current->magic != ALLOC_MAGIC) {
            printf("[MEM] Heap corruption detected at 0x%x\n", (uint32_t)current);
            return 0;
        }

        if (current->free && current->size >= requested_size) {
            split_block(current, requested_size);
            current->free = 0;
            memory_used += current->size;
            void *payload = (void *)((uint8_t *)current + sizeof(alloc_header_t));
            memset(payload, 0, current->size);
            return payload;
        }
        current = current->next;
    }

    printf("[MEM] malloc failed: %d bytes requested\n", (uint32_t)size);
    return 0;
}

static void coalesce_free_blocks(void)
{
    alloc_header_t *current = heap_head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(alloc_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void free(void *ptr)
{
    if (!ptr) {
        return;
    }

    alloc_header_t *block = (alloc_header_t *)((uint8_t *)ptr - sizeof(alloc_header_t));
    if (block->magic != ALLOC_MAGIC) {
        printf("[MEM] free ignored invalid pointer 0x%x\n", (uint32_t)ptr);
        return;
    }

    if (!block->free) {
        block->free = 1;
        if (memory_used >= block->size) {
            memory_used -= block->size;
        }
        coalesce_free_blocks();
    }
}

char *pmalloc(size_t size)
{
    (void)size;
    for (uint32_t i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (!pheap_bitmap[i]) {
            pheap_bitmap[i] = 1;
            uint32_t address = pheap_start + (i * PAGE_SIZE);
            memset((void *)address, 0, PAGE_SIZE);
            return (char *)address;
        }
    }
    printf("[MEM] pmalloc failed\n");
    return 0;
}

void print_memory_layout(void)
{
    uint32_t free_bytes = 0;
    alloc_header_t *current = heap_head;
    while (current) {
        if (current->free) {
            free_bytes += current->size;
        }
        current = current->next;
    }

    printf("[MEM] Layout:\n");
    printf("      heap start : 0x%x\n", heap_start);
    printf("      heap end   : 0x%x\n", heap_end);
    printf("      heap used  : %d bytes\n", memory_used);
    printf("      heap free  : %d bytes\n", free_bytes);
    printf("      pheap      : 0x%x - 0x%x\n", pheap_start, pheap_end);
}
