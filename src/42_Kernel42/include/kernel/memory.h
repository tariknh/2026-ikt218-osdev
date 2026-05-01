#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Represents an allocation region
 *
 * Status:
 * 0 - free
 * 1 - used
 */
typedef struct {
  uint8_t status;
  uint32_t size;
} alloc_t;

typedef struct {
  uint32_t last_alloc;  // memory location of last allocation
  uint32_t heap_end;    // location of end of heap
  uint32_t heap_begin;  // location of beginning of heap
  uint32_t memory_used; // total memory used

  uint32_t pheap_begin; // beginning of physical heap
  uint32_t pheap_end;   // ending of physical heap
  uint8_t* pheap_desc;  // not currently used
} memory_info_t;

/**
 * Initializes paging
 */
void init_paging();
/**
 * Translate virtual to physical address
 * @param virt virtual address
 * @param phys physicall address
 */
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

/**
 * Allocate one page
 * @return address of allocated page or NULL if failed
 */
char* alloc_page();
/**
 * Free a page
 * @param mem memory to free
 */
void free_page(void* mem);

/**
 * Set an area of memory to the provided value in pairs of 8 bits, word-wise
 * @param ptr memory address
 * @param value value to set
 * @param num number of 16-bit regions to set
 * @return memory address
 */
void* memset16(void* ptr, uint16_t value, size_t num);

/**
 * Print the current status of memory usage
 */
void print_memory_layout();
