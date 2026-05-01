#pragma once
#include <stdint.h>

#define PAGE_PRESENT 0x001
#define PAGE_RW 0x002
#define PAGE_USER 0x004
#define PAGE_ACCESSED 0x020
#define PAGE_DIRTY 0x040

#define PAGE_KERNEL_RW (PAGE_PRESENT | PAGE_RW)
#define PAGE_KERNEL_RO (PAGE_PRESENT)
#define PAGE_USER_RW (PAGE_PRESENT | PAGE_RW | PAGE_USER)
#define PAGE_USER_RO (PAGE_PRESENT | PAGE_USER)

#define KERNEL_VIRT_BASE 0xC0000000
#define USER_VIRT_BASE 0x00000000
#define USER_VIRT_TOP 0xBFFFFFFF
#define KERNEL_HEAP_VIRT 0xC0100000


// PAGING SETUP STUFF
#define PAGE_BITS 12
#define PAGE_SIZE (1UL << PAGE_BITS)
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PT_ENTRIES 1024

#define PTE_SHIFT PAGE_BITS
#define PGD_SHIFT 22 // page global directory shift
#define PT_INDEX_MASK 0x3FF

#define PGD_INDEX(addr) (((addr) >> PGD_SHIFT) & PT_INDEX_MASK)
#define PTE_INDEX(addr) (((addr) >> PTE_SHIFT) & PT_INDEX_MASK)

#define KERNEL_DIRECT_MAP_END 0x400000


/**
 * Initializes paging for user space
 * @param mb_info multiboot2 info struct
 */
void init_paging(void* mb_info);

/**
 * Map 4KB virtual page to physical frame
 * @param virt virtual address to map
 * @param phys physical frame addess
 *
 *  Allocates or find page table for virt, sets PTE (page table entry) tophys | PRESENT | RW.
 *  Invalidates TLB entry
 */
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

/**
 * Resolve virtual address to physical frame base + offset
 * @param virt  virtual address to lookup
 *
 * @return  physical address or 0 if unmapped
 */
uint32_t paging_get_phys(uint32_t virt);

// VMM VIRTUAL MEMORY MANAGER

/**
 * Map virtual page to physical frame with PTE flags
 * @param virt virtual address
 * @param phys physical frame address
 * @param flags PTE falgs (PAGE_PRESENT | PAGE_RW | PAGE_USER |  etc
 *  allocates or finds page table covering @virt. Sets PTE = phys | flags and invalidates TLB entry
 */
int vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);

/**
 * Unmap single virtual page from page tables
 * @param virt virtual address
 * Walks PD -> PT, clears PTE to 0 if PD presnet. Ignores missing. Invalidates TLB entry.
 */
int vmm_unmap_page(uint32_t virt);
/**
 * Map physical frame to user virtutal address
 * @param virt  user virtual address
 * @param phys  physical frame address
 * @param flags  PTE flags
 * Allocates new PT frame if PD entry missing. Sets PD + PTE with PAGE_USER.
 */
int vmm_map_user_page(uint32_t virt, uint32_t phys, uint32_t flags);

/** Implemented in assembly */

/**
 * Load CR3 with page directory location to point CPU to page directory physical base address
 */
extern void load_cr3(uint32_t addr);
/**
 * Enable paging by setting the PG bit in CR0
 */
extern void enable_paging(void);
/**
 * Invalidate page by using the invlpg instruction
 */
extern void invalidate_page(uint32_t addr);
