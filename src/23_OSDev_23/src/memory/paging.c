#include <stdint.h>
#include "paging.h"

// Page directory: 1024 entries, each covering 4MB
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

// One page table covering the first 4MB (1024 pages x 4KB)
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void init_paging() {
    // Identity-map the first 4MB: virtual address == physical address
    for (int i = 0; i < 1024; i++) {
        // Each page is 4KB, present + read/write
        first_page_table[i] = (i * 0x1000) | 3; // present + r/w
    }

    // Put our page table into the first entry of the page directory
    page_directory[0] = ((uint32_t)first_page_table) | 3; // present + r/w

    // Mark all other entries as not present
    for (int i = 1; i < 1024; i++) {
        page_directory[i] = 0x00000002; // not present, r/w
    }

    // Load the page directory address into CR3
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    // Enable paging by setting bit 31 of CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
