/*
 * This is a header file for a simple memory management library.
 * It includes declarations for functions related to memory allocation,
 * paging and memory manipulation (copying and setting).
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <libc/stdint.h> /* Include standard integer types */
#include <libc/stddef.h> /* Include standard definitions */
#include <libc/stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
 * Definition of a struct that represents a memory allocation.
 * It contains a status field (0 or 1) indicating if the memory
 * is currently allocated or not, and a size field indicating
 * the size of the allocated memory in bytes.
 */
typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

/* Init Kernel Memory */
void init_kernel_memory(uint32_t* kernel_end);


/* Function declarations for paging operations */
extern void init_paging(); /* Initializes paging */
extern void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys); /* Maps a virtual address to a physical address */

/* Function declarations for memory allocation */
extern char* pmalloc(size_t size); /* Allocates memory of given size with page alignment */
extern void* malloc(size_t size); /* Allocates memory of given size */
extern void free(void *mem); /* Frees memory previously allocated */

/* Function declarations for memory manipulation */
extern void* memcpy(void* dest, const void* src, size_t num ); /* Copies num bytes from src to dest */
extern void* memset (void * ptr, int value, size_t num ); /* Sets num bytes starting from ptr to value */
extern void* memset16 (void *ptr, uint16_t value, size_t num); /* Sets num bytes starting from ptr to a 16-bit value */

/* Other helper functions*/
void print_memory_layout(); // Printing heap begin, heap end, last allocated and memory used.
void kernel_main();        // Test C++ allocation using overloaded new.

// Heap getter functions for memory screen
uint32_t memory_get_heap_start();
uint32_t memory_get_heap_current();
uint32_t memory_get_heap_end();
uint32_t memory_get_heap_used();
uint32_t memory_get_heap_remaining();

// Kernel getter functions for memory screen
uint32_t memory_get_kernel_start();
uint32_t memory_get_kernel_end();
uint32_t memory_get_kernel_size();

// Getters for monitor paging screen
bool paging_is_enabled();
uint32_t paging_get_page_size();
uint32_t paging_get_mapped_start();
uint32_t paging_get_mapped_end();
uint32_t paging_get_cr3();
uint32_t paging_get_page_directory_address();
uint32_t paging_get_first_page_table_address();
uint32_t paging_get_last_page_table_address();

#ifdef __cplusplus
}
#endif

#endif