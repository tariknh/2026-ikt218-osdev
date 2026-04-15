#include "memory.h"
#include <stdint.h>

static uint32_t* page_directory = (uint32_t*)0x400000;

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t pd_index = virt >> 22;
    uint32_t* page_table = (uint32_t*)(phys + 0x1000);

    uint32_t i;
    for (i = 0; i < 1024; i++) {
        page_table[i] = (phys + (i * 0x1000)) | 3;
    }

    page_directory[pd_index] = ((uint32_t)page_table) | 3;
}

void init_paging(void)
{
    uint32_t i;
    for (i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    paging_map_virtual_to_phys(0, 0);
    paging_map_virtual_to_phys(0x400000, 0x400000);

    __asm__ volatile (
        "mov %0, %%cr3\n"
        :
        : "r"(page_directory)
    );

    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}
