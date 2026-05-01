
#include "memory.h"

static uint32_t* page_directory = 0;   // Define a pointer to the page directory and initialize it to zero
static uint32_t page_dir_loc = 0;      // Define the location of the page directory and initialize it to zero
static uint32_t* last_page = 0;        // Define a pointer to the last page and initialize it to zero

/* really simple paging setup for now
 * first 8 MB is for kernel stuff
 * normal heap is below 4 MB
 * page directory and page tables start at 4 MB
 */

// this maps one 4 MB virtual region to one 4 MB physical region
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint16_t id = virt >> 22;        // which page-directory entry this virtual region belongs to
    for(int i = 0; i < 1024; i++)   // 1024 entries and each one maps 4 KB, so that becomes 4 MB total
    {
        last_page[i] = phys | 3;    // map this page and mark it present + writable
        phys += 4096;               // move to the next 4 KB physical page
    }
    page_directory[id] = ((uint32_t)last_page) | 3;  // connect this page table into the page directory
    last_page = (uint32_t *)(((uint32_t)last_page) + 4096); // next free 4 KB page can be another page table
}

// now tell the CPU where the page directory is and turn paging on
void paging_enable()
{
    asm volatile("mov %%eax, %%cr3": :"a"(page_dir_loc)); // CR3 should point to the page directory
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");  // set the paging bit
    asm volatile("mov %eax, %cr0");
}

// set up simple identity mapping so the kernel can still use the same addresses after paging is enabled
void init_paging()
{
    printf("Setting up paging\n");
    page_directory = (uint32_t*)0x400000;      // put the page directory at 4 MB
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t *)0x404000;         // first free page after the directory is used for page tables
    for(int i = 0; i < 1024; i++)
    {
        page_directory[i] = 0 | 2;            // writable, but not present until we actually map something there
    }
    paging_map_virtual_to_phys(0, 0);                // identity map 0 MB - 4 MB
    paging_map_virtual_to_phys(0x400000, 0x400000);  // identity map 4 MB - 8 MB
    paging_enable();
    printf("Paging was successfully enabled!\n");
}
