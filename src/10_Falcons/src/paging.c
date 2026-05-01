#include <kernel/memory.h>
#include <kernel/terminal.h>

#define PAGE_PRESENT 0x1u
#define PAGE_RW      0x2u
#define PAGE_SIZE    4096u

static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t first_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t second_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint32_t directory_index = virt >> 22;
    uint32_t table_index = (virt >> 12) & 0x3FFu;
    uint32_t *table = 0;

    if (directory_index == 0) {
        table = first_page_table;
    } else if (directory_index == 1) {
        table = second_page_table;
    } else {
        return;
    }

    table[table_index] = (phys & 0xFFFFF000u) | PAGE_PRESENT | PAGE_RW;
    page_directory[directory_index] = ((uint32_t)table) | PAGE_PRESENT | PAGE_RW;
}

void init_paging(void)
{
    printf("[PAGING] Identity mapping first 8 MiB...\n");

    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i] = PAGE_RW;
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
        second_page_table[i] = ((1024u + i) * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }

    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
    page_directory[1] = ((uint32_t)second_page_table) | PAGE_PRESENT | PAGE_RW;

    __asm__ volatile ("mov %0, %%cr3" :: "r"((uint32_t)page_directory));

    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0));

    printf("[PAGING] Paging enabled.\n");
}
