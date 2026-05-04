#include "paging.h"
#include "terminal.h"
#include <libc/stdint.h>
#include <libc/stdbool.h>

// Basic page flags for the first mapping
#define PAGE_FLAG_PRESENT 0x001
#define PAGE_FLAG_RW      0x002
// Page size for the identity map
#define PAGE_SIZE_4K      0x1000
// One page table has 1024 entries
#define ENTRIES_PER_TABLE 1024
// Bit that turns paging on in CR0
#define CR0_PG_BIT        0x80000000u

// Page directory and first page table are aligned for the CPU
static uint32_t page_directory[ENTRIES_PER_TABLE] __attribute__((aligned(PAGE_SIZE_4K)));
static uint32_t first_page_table[ENTRIES_PER_TABLE] __attribute__((aligned(PAGE_SIZE_4K)));
static bool paging_ready = false;

// Load CR3 with the page directory address
static inline void load_cr3(uint32_t phys_addr) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(phys_addr) : "memory");
}

// Read CR0 so we can check paging state
static inline uint32_t read_cr0(void) {
    uint32_t value;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(value));
    return value;
}

// Write CR0 to turn paging on
static inline void write_cr0(uint32_t value) {
    __asm__ volatile ("mov %0, %%cr0" : : "r"(value) : "memory");
}

// Read CR3 for the self test
static inline uint32_t read_cr3(void) {
    uint32_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}

// Set up a simple identity map for low memory
void init_paging(void) {
    if (paging_ready) {
        return;
    }

    // Clear the directory and map the first 4 MiB
    for (uint32_t i = 0; i < ENTRIES_PER_TABLE; i++) {
        page_directory[i] = 0;
        first_page_table[i] = (i * PAGE_SIZE_4K) | PAGE_FLAG_PRESENT | PAGE_FLAG_RW;
    }

    // Point the first directory entry at the first table
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_FLAG_PRESENT | PAGE_FLAG_RW;

    // Load the new paging structures and enable paging
    load_cr3((uint32_t)page_directory);
    write_cr0(read_cr0() | CR0_PG_BIT);

    paging_ready = true;
    terminal_printf("Paging enabled (identity map 0x0-0x3fffff)\n");
}

// Check that paging is active and the directory is loaded
bool paging_self_test(void) {
    uint32_t cr0 = read_cr0();
    uint32_t cr3 = read_cr3();
    bool pg_enabled = (cr0 & CR0_PG_BIT) != 0;

    volatile uint16_t *vga = (volatile uint16_t *)0xB8000u;
    uint16_t sample = vga[0];

    bool cr3_matches = ((cr3 & 0xFFFFF000u) == ((uint32_t)page_directory & 0xFFFFF000u));

    terminal_printf("Paging test: CR0=0x%x CR3=0x%x VGA[0]=0x%x\n",
                    cr0, cr3, (uint32_t)sample);
    terminal_printf("Paging test: PG=%s CR3-match=%s ready=%s\n",
                    pg_enabled ? "ON" : "OFF",
                    cr3_matches ? "YES" : "NO",
                    paging_ready ? "YES" : "NO");

    return pg_enabled && cr3_matches && paging_ready;
}
