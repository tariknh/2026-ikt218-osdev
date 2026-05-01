#include <kernel/memory.h>
#include <kernel/terminal.h>
#include <libc/stdint.h>

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys) {
    uint16_t id = virt >> 22;

    // Build one page table that maps a full 4 MiB region in 4 KiB steps.
    for(int i = 0; i < 1024; i++) {
        last_page[i] = phys | 3;
        phys += 4096;
    }

    page_directory[id] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t*)(((uint32_t)last_page) + 4096);
}

void paging_enable() {
    // cr3 points at the page directory; bit 31 in cr0 turns paging on.
    asm volatile("mov %%eax, %%cr3": :"a"(page_dir_loc));
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}

void InitPaging(void) {
    TerminalWriteString("Setting up paging\n");

    // Reserve 0x400000 for the page directory and place page tables right
    // after it. The addresses are hard-coded because this early kernel uses
    // a fixed simple layout before any more flexible VM scheme exists.
    page_directory = (uint32_t*)0x400000;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t*)0x404000;

    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | 2;
    }

    // Identity-map the first 8 MiB so the kernel can keep using the same
    // addresses immediately before and after paging is turned on.
    paging_map_virtual_to_phys(0, 0);
    paging_map_virtual_to_phys(0x400000, 0x400000);
    
    paging_enable();
    TerminalWriteString("Paging was successfully enabled!\n");
}
