#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <libc/stdint.h>

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void InitKernelMemory(uint32_t* kernel_end);

void InitPaging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

char* pmalloc(size_t size);
void* malloc(size_t size);
void free(void *mem);

void* memcpy(void* dest, const void* src, size_t num);
void* memset (void* ptr, int value, size_t num);
void* memset16 (void* ptr, uint16_t value, size_t num);

void PrintMemoryLayout(void);

#endif