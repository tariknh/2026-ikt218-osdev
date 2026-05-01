#include <stdlib.h>
#include <string.h>

#include "kernel/log.h"
#include "kernel/memory.h"
#include "kernel/paging.h"
#include "kernel/pmm.h"
#include "kernel/panic.h"


memory_info_t memory_info;

static uint32_t heap_next = 0x08000000;

// Free a block obtained by malloc
// mem is either NULL or pointer
void free(void* mem) {
    if (!mem)
        return;
    // remove header size from pointer to find header start, since the layout is [alloc_t header] [ user data ...]
    alloc_t* header = (alloc_t*)((uint32_t)mem - sizeof(alloc_t));

    uint32_t size = header->size;
    uint32_t block_start = (uint32_t)header;
    header->status = 0;

    // compute how many pages the allocation occupied, and unmap each of them
    size_t total_size = size + sizeof(alloc_t);
    size_t num_pages = (total_size + 4095) / 4096;
    if (num_pages == 0) num_pages = 1;

    for (size_t i = 0; i < num_pages; i++) {
        uint32_t virt = block_start + (i * 4096);
        uint32_t phys = paging_get_phys(virt);
        if (phys) {
            pmm_free_frame(phys);
            vmm_unmap_page(virt);
        }
    }

    memory_info.memory_used -= size;
}

// allocate a block of ATLEAST `size` bytes
// returns a page aligned pointer right after the alloc_t header for the allocation
void* malloc(size_t size) {
    if (!size)
        return NULL;

    // allocate atleast one page,  rounding up to cover header + size
    size_t total_size = size + sizeof(alloc_t);
    size_t num_pages = (total_size + 4095) / 4096;
    if (num_pages == 0) num_pages = 1;
    size_t actual_size = num_pages * 4096;

    uint32_t virt = heap_next;
    heap_next += actual_size;

    // map each of the pages, from PMM into the virtual heap range
    for (size_t i = 0; i < num_pages; i++) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) {
            kernel_panic("malloc: out of memory\n");
        }
        vmm_map_page(virt + (i * 4096), phys, PAGE_USER_RW);
    }

    // Mark block as allocated and initialize the header
    // zero the user data portion before returning
    alloc_t* header = (alloc_t*)virt;
    header->status = 1;
    header->size = size;

    memory_info.memory_used += size;
    memset((void*)(virt + sizeof(alloc_t)), 0, size);

    return (void*)(virt + sizeof(alloc_t));
}
