#include <libc/stdint.h>

#include "paging.h"
#include "terminal.h"

#define PAGE_PRESENT 0x001u
#define PAGE_WRITABLE 0x002u
#define PAGE_SIZE 4096u
#define PAGE_TABLE_ENTRIES 1024u
#define PAGE_TABLE_COUNT 4u /* Identity-map first 16 MiB */

static uint32_t page_directory[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint32_t page_tables[PAGE_TABLE_COUNT][PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void load_page_directory(uint32_t* page_directory_address) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory_address));
}

static void enable_paging_hw(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u; /* PG bit */
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

void init_paging(void) {
    for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    uint32_t physical_address = 0;
    for (uint32_t table = 0; table < PAGE_TABLE_COUNT; table++) {
        for (uint32_t entry = 0; entry < PAGE_TABLE_ENTRIES; entry++) {
            page_tables[table][entry] = physical_address | PAGE_PRESENT | PAGE_WRITABLE;
            physical_address += PAGE_SIZE;
        }

        page_directory[table] = ((uint32_t)page_tables[table]) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    load_page_directory(page_directory);
    enable_paging_hw();

    terminal_write("Paging initialized with identity mapping for first 16 MiB.\n");
}