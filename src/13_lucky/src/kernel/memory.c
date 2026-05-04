#include "kernel/memory.h"

void init_paging() {
    // Place the page directory in a free memory area
    uint32_t *page_directory = (uint32_t *) 0x400000;
    uint32_t page_dir_loc = (uint32_t) page_directory;
    uint32_t *last_page = (uint32_t *) 0x404000;

    // Mark every page directory entry as not present until it is mapped below
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | 2;
    }

    // Identity map the first 8 MB so early kernel addresses keep working
    for (uint32_t virt = 0; virt <= 0x400000; virt += 0x400000) {
        uint32_t phys = virt;
        uint16_t id = virt >> 22;

        // Fill a page table with 4 KB pages
        for (int i = 0; i < 1024; i++) {
            last_page[i] = phys | 3;
            phys += 4096;
        }

        page_directory[id] = (uint32_t) last_page | 3;
        last_page = (uint32_t *) ((uint32_t) last_page + 4096);
    }

    // Load the page directory into CR3, then set the paging bit in CR0
    asm volatile("mov %%eax, %%cr3": :"a"(page_dir_loc));
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}
