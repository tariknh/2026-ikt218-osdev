#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h>
#include <libc/stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void init_kernel_memory(uint32_t* kernel_end);

void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

char* pmalloc(size_t size);
void* malloc(size_t size);
void free(void* mem);

void* memcpy(void* dest, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
void* memset16(void* ptr, uint16_t value, size_t num);

void print_memory_layout(void);

#ifdef __cplusplus
}
#endif

#endif
