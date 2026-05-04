/*
 * paging.c - Enable basic paging
 *
 * Page directory entry / page table entry flag bits used here:
 *   bit 0 (P)  Present
 *   bit 1 (RW) Read/Write
 *
 * Strategy: identity-map the first 4 MiB of physical memory with one page
 * table, then load the page directory and turn on paging.  After this
 * function returns, every linear address in [0, 4 MiB) maps to the same
 * physical address, so the kernel and the heap continue to work unchanged.
 */

#include <paging.h>
#include <libc/stdint.h>

#define PAGE_PRESENT    0x1
#define PAGE_RW         0x2

/* The page directory and the first page table must each be aligned to a
   4 KiB boundary.  We allocate them statically with __attribute__((aligned)). */
static uint32_t page_directory[1024]   __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void init_paging(void) {
    /* Build the first page table: entry i points at physical 4 KiB * i. */
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;
    }

    /* Build the page directory: entry 0 points at our page table; the rest
       are marked not-present so a stray access there will fault. */
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
    for (uint32_t i = 1; i < 1024; i++) {
        page_directory[i] = 0;
    }

    /* Load the page directory address into CR3 and set the PG bit (bit 31)
       in CR0 to enable paging. */
    __asm__ volatile (
        "mov %0, %%cr3\n\t"
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        :
        : "r" (page_directory)
        : "eax"
    );
}
