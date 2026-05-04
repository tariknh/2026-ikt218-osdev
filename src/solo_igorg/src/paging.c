#include <paging.h>
#include <terminal.h>
#include <libc/stdint.h>

/*
 * Paging constants.
 */
#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

/*
 * Page flags.
 *
 * bit 0 = present
 * bit 1 = writable
 */
#define PAGE_PRESENT 0x1
#define PAGE_WRITABLE 0x2

/*
 * One page directory and one page table.
 */
static uint32_t page_directory[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint32_t first_page_table[PAGE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void load_page_directory(uint32_t* page_directory_address)
{
    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory_address));
}

static void enable_paging(void)
{
    uint32_t cr0;

    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));

    /*
     * Sets bit 31 in CR0.
     * This enables paging.
     */
    cr0 |= 0x80000000;

    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

void init_paging(void)
{
    /*
     * Clears the page directory.
     */
    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    /*
     * Maps first 4 MiB of memory.
     * Entry i maps virtual address i * 4096 to physical address i * 4096.
     */
    for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    /*
     * Points first page directory entry to first page table.
     */
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_WRITABLE;

    /*
     * Loads page directory into CR3 and enable paging in CR0.
     */
    load_page_directory(page_directory);
    enable_paging();

    terminal_write("Paging initialized: identity-mapped first 4 MiB\n");
}
