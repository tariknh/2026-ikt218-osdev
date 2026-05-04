#pragma once
#include "cdefs.h"
#include "stddef.h"
#include "stdint.h"

EXTERN_C_BEGIN

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void init_kernel_memory(uint32_t *kernel_end);

char *pmalloc(size_t size);

void pfree(void *mem);

void *malloc(size_t size);

void free(void *mem);

void *memcpy(void *dest, const void *src, size_t num);

void *memset(void *ptr, int value, size_t num);

void *memset16(void *ptr, uint16_t value, size_t num);

void init_paging();

void print_memory_layout();

EXTERN_C_END
