#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stddef.h>
#include <libc/stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void init_kernel_memory(uint32_t *kernel_end);
void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);
void print_memory_layout(void);

void *malloc(size_t size);
void free(void *mem);

#ifdef __cplusplus
}
#endif

#endif
