#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

// Turns paging on and creates the first page tables
void init_paging(void);

// Maps one 4 MB virtual region to one 4 MB physical region
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

#ifdef __cplusplus
}
#endif

#endif