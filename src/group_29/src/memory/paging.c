#include "heap.h"

static uint32_t kernel_page_directory[1024] __attribute__((aligned(PAGE_SIZE_BYTES)));
static uint32_t kernel_page_table0[1024] __attribute__((aligned(PAGE_SIZE_BYTES)));

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys) {
    uint32_t directory_index = virt >> 22;
    uint32_t table_index = (virt >> 12) & 0x3FFU;
    uint32_t* page_table;

    if ((kernel_page_directory[directory_index] & 0x1U) == 0U) {
        return;
    }

    page_table = (uint32_t*)(kernel_page_directory[directory_index] & 0xFFFFF000U);
    page_table[table_index] = (phys & 0xFFFFF000U) | 0x3U;

    __asm__ __volatile__("invlpg (%0)" : : "r"((void*)virt) : "memory");
}

void init_paging(void) {
    uint32_t index;
    uint32_t cr0;

    for (index = 0U; index < 1024U; ++index) {
        kernel_page_directory[index] = 0x00000002U;
        kernel_page_table0[index] = (index * PAGE_SIZE_BYTES) | 0x3U;
    }

    kernel_page_directory[0] = ((uint32_t)kernel_page_table0) | 0x3U;

    __asm__ __volatile__("mov %0, %%cr3" : : "r"(kernel_page_directory) : "memory");

    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000U;
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0) : "memory");
}
