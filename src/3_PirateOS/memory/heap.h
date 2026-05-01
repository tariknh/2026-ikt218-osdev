#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include "libc/stddef.h"
#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

// Sets up the kernel heap after the kernel image in memory
void init_kernel_memory(uint32_t *kernel_end);

// Allocates whole aligned pages from the page heap
char *pmalloc(size_t size);
void pfree(void *mem);

// Allocates normal heap memory
void *malloc(size_t size);
void free(void *mem);

// Prints heap information for debugging
void print_memory_layout(void);

#ifdef __cplusplus
}
#endif

#endif