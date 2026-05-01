#include "memory/paging.h"
#include "libc/memory.h"
#include "libc/stdio.h"

/*
 * This file turns on paging in a simple way
 * It builds a page directory, creates page tables, and identity-maps the first 8 MB
 */

#define PAGE_DIRECTORY_ADDR 0x400000
#define FIRST_PAGE_TABLE_ADDR 0x401000
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024

static uint32_t *page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t *last_page = 0;

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t page_directory_index = virt >> 22;
    uint32_t i;

    // Fill one page table with 1024 page entries
    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        last_page[i] = (phys + (i * PAGE_SIZE)) | 3;
    }

    // Connect that page table to the correct page directory slot
    page_directory[page_directory_index] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t *)((uint32_t)last_page + PAGE_SIZE);
}

static void paging_enable(void)
{
    uint32_t cr0;

    // Tell the CPU where the page directory is and then enable paging
    asm volatile("mov %0, %%cr3" : : "r"(page_dir_loc));
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void init_paging(void)
{
    uint32_t i;

    //printf("Setting up paging\n");

    // Place the page directory and first page table at known addresses
    page_directory = (uint32_t *)PAGE_DIRECTORY_ADDR;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t *)FIRST_PAGE_TABLE_ADDR;

    // Start with an empty page directory
    memset(page_directory, 0, PAGE_SIZE);
    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        page_directory[i] = 2;
    }

    // Identity-map the first 8 MB so the kernel keeps working after paging is enabled
    paging_map_virtual_to_phys(0, 0);
    paging_map_virtual_to_phys(0x400000, 0x400000);
    paging_enable();

    //printf("Paging enabled\n");
}