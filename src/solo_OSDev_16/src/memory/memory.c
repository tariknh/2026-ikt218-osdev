#include "memory.h"

// Getter values for monitor paging screen
static volatile bool paging_enabled = false;

#define PAGE_SIZE 4096

static uint32_t paging_mapped_start = 0;
static uint32_t paging_mapped_end = 0;
static uint32_t paging_mapped_initialized = 0;

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

// Map one 4 MiB virtual memory area to physical memory
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    // Upper 10 bits select the page directory entry
    uint16_t id = virt >> 22;

    // Store the first mapped virtual address
    if (!paging_mapped_initialized) {
        paging_mapped_start = virt;
        paging_mapped_initialized = 1;
    }

    // Fill one page table with 1024 entries of 4 KiB each
    for (int i = 0; i < 1024; i++)
    {
        last_page[i] = phys | 3;   // Present + writable
        phys += PAGE_SIZE;
    }

    // Store the last mapped virtual address for this mapped 4 MiB area
    paging_mapped_end = virt + (1024 * PAGE_SIZE) - 1;

    // Store the page table address in the page directory
    // and write permission set
    page_directory[id] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t *)(((uint32_t)last_page) + PAGE_SIZE);
}

// Load the page directory and enable paging in the CPU
static void paging_enable()
{
    // Load the physical address of the page directory into CR3
    asm volatile("mov %0, %%cr3" : : "r"(page_dir_loc));

    // Read CR0, set the paging bit, and write it back
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    paging_enabled = true;
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

bool paging_is_enabled()
{
    return paging_enabled;
}

uint32_t paging_get_page_size()
{
    return PAGE_SIZE;
}

uint32_t paging_get_mapped_start()
{
    return paging_mapped_start;
}

uint32_t paging_get_mapped_end()
{
    return paging_mapped_end;
}

uint32_t paging_get_cr3()
{
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

uint32_t paging_get_page_directory_address()
{
    return page_dir_loc;
}

uint32_t paging_get_first_page_table_address()
{
    return page_dir_loc + PAGE_SIZE;
}

uint32_t paging_get_last_page_table_address()
{
    return page_dir_loc + (PAGE_SIZE * 2);
}