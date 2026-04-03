#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h>
#include <libc/stddef.h>

/*
 * Allocation metadata struct stored before each malloc'd block.
 * status: 1 = allocated, 0 = free
 * size: size of the user data (not including this header)
 */
typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

/* Kernel memory manager init */
void init_kernel_memory(uint32_t* kernel_end);

/* Paging */
void init_paging();
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

/* Memory allocation */
char* pmalloc(size_t size);
void  pfree(void *mem);
void* malloc(size_t size);
void  free(void *mem);

/* Memory utilities */
void* memcpy(void* dest, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
void* memset16(void *ptr, uint16_t value, size_t num);

/* Debug */
void print_memory_layout();

#endif /* MEMORY_H */
