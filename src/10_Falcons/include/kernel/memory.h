#ifndef UIAOS_KERNEL_MEMORY_H
#define UIAOS_KERNEL_MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct alloc_header {
    uint32_t magic;
    uint32_t size;
    uint32_t free;
    struct alloc_header *next;
} alloc_header_t;

void init_kernel_memory(uint32_t *kernel_end);
void print_memory_layout(void);

void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

void *malloc(size_t size);
void free(void *ptr);
char *pmalloc(size_t size);

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *ptr, int value, size_t count);
void *memset16(void *ptr, uint16_t value, size_t count);

#endif
