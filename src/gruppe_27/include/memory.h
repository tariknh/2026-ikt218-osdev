#ifndef MEMORY_H
#define MEMORY_H
 
#include <stdint.h>
#include <stddef.h>
#include "multiboot2.h"

 

extern uint32_t end;
extern uint32_t g_mb2_info;
 
void  init_kernel_memory(uint32_t *kernel_end);
 
void *malloc(size_t size);
void  free(void *ptr);
void *memmove(void* dest, const void* src, size_t n);
 
void init_paging();
 

void print_memory_layout(uint32_t mb2_info_addr);
void print_heap_info();


 
#endif // MEMORY_H