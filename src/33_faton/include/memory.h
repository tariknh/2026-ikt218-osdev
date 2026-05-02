#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h>
#include <libc/stddef.h>

typedef struct {
    uint8_t  status;
    uint32_t size;
} alloc_t;

void  init_kernel_memory(uint32_t* kernel_end);
void  print_memory_layout(void);
void* malloc(size_t size);
void  free(void* mem);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t count);
void  init_paging(void);

#endif