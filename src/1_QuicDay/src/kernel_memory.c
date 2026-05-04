#include "kernel_memory.h"

#include "libc/stdbool.h"
#include "libc/stdio.h"

#define KERNEL_HEAP_SIZE (4 * 1024 * 1024)
#define PAGE_SIZE 4096

typedef struct block_header {
    size_t size;
    bool is_free;
    struct block_header* next;
} block_header_t;

static uint8_t* g_heap_start = 0;
static uint8_t* g_heap_end = 0;
static uint8_t* g_kernel_end = 0;
static block_header_t* g_free_list = 0;

static uint32_t align_up_u32(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1U) & ~(alignment - 1U);
}

static size_t align_up_size(size_t value, size_t alignment) {
    return (value + alignment - 1U) & ~(alignment - 1U);
}

void init_kernel_memory(void* kernel_end) {
    g_kernel_end = (uint8_t*)kernel_end;
    uint32_t heap_begin = align_up_u32((uint32_t)kernel_end, 16);

    g_heap_start = (uint8_t*)heap_begin;
    g_heap_end = g_heap_start + KERNEL_HEAP_SIZE;

    g_free_list = (block_header_t*)g_heap_start;
    g_free_list->size = KERNEL_HEAP_SIZE - sizeof(block_header_t);
    g_free_list->is_free = true;
    g_free_list->next = 0;
}

void* malloc(size_t size) {
    if (size == 0 || g_free_list == 0) {
        return 0;
    }

    size = align_up_size(size, 16);

    block_header_t* current = g_free_list;
    while (current != 0) {
        if (current->is_free && current->size >= size) {
            size_t remaining = current->size - size;

            if (remaining > sizeof(block_header_t) + 16) {
                block_header_t* split = (block_header_t*)((uint8_t*)(current + 1) + size);
                split->size = remaining - sizeof(block_header_t);
                split->is_free = true;
                split->next = current->next;

                current->size = size;
                current->next = split;
            }

            current->is_free = false;
            return (void*)(current + 1);
        }

        current = current->next;
    }

    return 0;
}

void free(void* ptr) {
    if (ptr == 0) {
        return;
    }

    block_header_t* block = ((block_header_t*)ptr) - 1;
    block->is_free = true;

    block_header_t* current = g_free_list;
    while (current != 0 && current->next != 0) {
        uint8_t* end_of_current = (uint8_t*)(current + 1) + current->size;

        if (current->is_free && current->next->is_free && end_of_current == (uint8_t*)current->next) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
            continue;
        }

        current = current->next;
    }
}

__attribute__((aligned(PAGE_SIZE))) static uint32_t page_directory[1024];
__attribute__((aligned(PAGE_SIZE))) static uint32_t first_page_table[1024];

void init_paging(void) {
    for (uint32_t i = 0; i < 1024; ++i) {
        first_page_table[i] = (i * PAGE_SIZE) | 0x3;
        page_directory[i] = 0x00000002;
    }

    page_directory[0] = ((uint32_t)first_page_table) | 0x3;

    asm volatile("mov %0, %%cr3" : : "r"((uint32_t)page_directory));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void print_memory_layout(void) {
    printf("Memory layout:\n");
    printf("  kernel end: %p\n", g_kernel_end);
    printf("  heap start: %p\n", g_heap_start);
    printf("  heap end  : %p\n", g_heap_end);
    printf("  heap size : %u bytes\n", (uint32_t)(g_heap_end - g_heap_start));
}
