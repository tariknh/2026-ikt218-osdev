#include "memory.h"

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

// Map one 4 MiB virtual memory area to physical memory
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    // Upper 10 bits select the page directory entry
    uint16_t id = virt >> 22;

    // Fill one page table with 1024 entries of 4 KiB each
    for (int i = 0; i < 1024; i++)
    {
        last_page[i] = phys | 3;   // Present + writable
        phys += 4096;
    }

    // Store the page table address in the page directory
    // and write permission set
    page_directory[id] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t *)(((uint32_t)last_page) + 4096);
}

// Load the page directory and enable paging in the CPU
static void paging_enable()
{
    // Load the physical address of the page into CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_dir_loc));

    // Read CR0, set the paging bit, and write it back
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

// Initialize identity paging for the first 8 MiB
void init_paging()
{
    page_directory = (uint32_t*)0x400000;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t*)0x404000;

    // Mark all page directory entries as not present
    for (int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0 | 2;
    }

    // Identity-map the first 4 MiB: virtual 0x000000 maps to physical 0x000000
    paging_map_virtual_to_phys(0, 0); 
    // Identity-map the next 4 MiB: virtual 0x400000 maps to physical 0x400000                  
    paging_map_virtual_to_phys(0x400000, 0x400000);

    paging_enable();
}
