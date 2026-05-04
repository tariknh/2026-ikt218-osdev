#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void init_kernel_memory(uint32_t* kernel_end);
void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);
void* pmalloc(uint32_t size);
void* malloc(uint32_t size);
void free(void* ptr);
void* memcpy(void* dest, const void* src, uint32_t n);
void* memset(void* dest, uint8_t val, uint32_t n);
void* memset16(void* dest, uint16_t val, uint32_t n);
void print_memory_layout(void);

#endif
