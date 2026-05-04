#include "memory.h"
#include "terminal.h"
 

// ─── Constants ───────────────────────────────────────────────────────────────
 
#define PAGE_SIZE   4096u
#define PD_PRESENT  0x1   // Page directory entry: present
#define PD_RW       0x2   // Page directory entry: read+write
#define PT_PRESENT  0x1   // Page table entry: present
#define PT_RW       0x2   // Page table entry: read+write
 
// ─── Helpers ─────────────────────────────────────────────────────────────────
 
static inline uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + align - 1) & ~(align - 1);
}
 
static void print_hex32(uint32_t v) {
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 9; i >= 2; i--) {
        uint32_t n = v & 0xF;
        buf[i] = (n < 10) ? ('0' + n) : ('A' + n - 10);
        v >>= 4;
    }
    buf[10] = '\0';
    terminal_write(buf);
}
 
static void print_hex64(uint64_t v) {
    print_hex32((uint32_t)(v >> 32));
    terminal_write("_");
    print_hex32((uint32_t)(v & 0xFFFFFFFF));
}
 
// ─── Page structures ─────────────────────────────────────────────────────────
//
// Declared static so they live in .bss and are guaranteed to be
// page-aligned by the __attribute__.  They must NOT be allocated with
// malloc() because paging is set up before (or alongside) the heap.
 
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_table_0  [1024] __attribute__((aligned(4096)));
 
// ─── init_paging ─────────────────────────────────────────────────────────────
 
void init_paging() {
    // Clear both structures.
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;
        page_table_0[i]   = 0;
    }
 
    // Identity-map the first 4 MiB: page N maps to physical address N*4096.
    for (uint32_t i = 0; i < 1024; i++) {
        page_table_0[i] = (i * PAGE_SIZE) | PT_PRESENT | PT_RW;
    }
 
    // Wire the first page-directory entry to our page table.
    page_directory[0] = (uint32_t)page_table_0 | PD_PRESENT | PD_RW;
 
    // Point CR3 at the page directory.
    __asm__ __volatile__("mov %0, %%cr3" : : "r"((uint32_t)page_directory));
 
    // Set CR0.PG (bit 31) to enable paging.
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000u;
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0));
 
    terminal_write("[paging] identity map active (0x00000000 - 0x003FFFFF)\n");
}
 
// ─── print_memory_layout ─────────────────────────────────────────────────────
 
static const char *mmap_type_str(uint32_t type) {
    switch (type) {
        case MULTIBOOT_MEMORY_AVAILABLE:        return "Available";
        case MULTIBOOT_MEMORY_RESERVED:         return "Reserved";
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE: return "ACPI Reclaimable";
        case MULTIBOOT_MEMORY_NVS:              return "ACPI NVS";
        case MULTIBOOT_MEMORY_BADRAM:           return "Bad RAM";
        default:                                return "Unknown";
    }
}
 
void print_memory_layout(uint32_t mb2_info_addr) {
    if (!mb2_info_addr) {
        terminal_write("[memmap] no multiboot2 info pointer\n");
        return;
    }
 
    terminal_write("\n=== Physical Memory Map ===\n");
    terminal_write("Address              Length               Type\n");
    terminal_write("--------------------------------------------------\n");
 
    uint8_t *tag_ptr  = (uint8_t *)(mb2_info_addr + 8); // skip size + reserved
    uint8_t *info_end = (uint8_t *)(mb2_info_addr +
                            *(uint32_t *)mb2_info_addr);
    int found = 0;
 
    while (tag_ptr < info_end) {
        struct multiboot_tag *tag = (struct multiboot_tag *)tag_ptr;
        if (tag->type == MULTIBOOT_TAG_TYPE_END) break;
 
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            struct multiboot_tag_mmap *mmap_tag =
                (struct multiboot_tag_mmap *)tag;
            multiboot_memory_map_t *entry = mmap_tag->entries;
            uint8_t *mmap_end = (uint8_t *)tag + tag->size;
 
            while ((uint8_t *)entry < mmap_end) {
                print_hex64(entry->addr);
                terminal_write("  ");
                print_hex64(entry->len);
                terminal_write("  ");
                terminal_write(mmap_type_str(entry->type));
                terminal_write("\n");
                found = 1;
                entry = (multiboot_memory_map_t *)
                    ((uint8_t *)entry + mmap_tag->entry_size);
            }
        }
 
        tag_ptr += align_up(tag->size, MULTIBOOT_TAG_ALIGN);
    }
 
    if (!found)
        terminal_write("(no mmap tag found)\n");
 
    terminal_write("=== End of Memory Map ===\n\n");
}