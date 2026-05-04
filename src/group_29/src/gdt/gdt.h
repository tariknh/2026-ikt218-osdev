#pragma once
#include "libc/stdint.h"

/**
 * @brief One 8-byte entry in the Global Descriptor Table (GDT).
 *
 * This structure matches the x86 protected mode segment descriptor layout.
 * It is marked packed so the compiler does not insert padding between fields.
 */
struct gdt_entry {
    //Lower 16 bits of the segment limit.
    uint16_t limit_low;

    //Lower 16 bits of the segment base address.
    uint16_t base_low;

    //Middle 8 bits of the segment base address.    
    uint8_t  base_middle;

    //Access byte describing privilege, type, and presence.
    uint8_t  access;

    //Upper 4 bits of the limit and upper 4 flag bits.
    uint8_t  granularity;

    //Upper 8 bits of the segment base address.
    uint8_t  base_high;
} __attribute__((packed));

/**
 * @brief Pointer structure used by the lgdt instruction.
 *
 * limit is the size of the GDT in bytes minus one.
 * address is the linear address of the first GDT entry.
 */
struct gdt_pointer {
    //Size of the GDT in bytes minus one.
    uint16_t limit;

    //Address of the first GDT entry.
    uint32_t address;
} __attribute__((packed));

/**
 * @brief Creates a single GDT entry from base, limit, access byte, and flags.
 *
 * @param base Base address of the segment.
 * @param limit Segment limit.
 * @param access_byte Access byte for the descriptor.
 * @param flags Granularity and size flags in the upper nibble.
 * @return A fully initialized GDT entry.
 */
struct gdt_entry create_gdt_entry(uint32_t base, uint32_t limit, uint8_t access_byte, uint8_t flags);

/**
 * @brief Creates the pointer structure required by lgdt.
 *
 * @param start Pointer to the first GDT entry.
 * @param entries Number of entries in the table.
 * @return A GDT pointer suitable for loading with lgdt.
 */
struct gdt_pointer create_gdt_pointer(struct gdt_entry *start, uint32_t entries);

/**
 * @brief Initializes and loads the kernel GDT.
 *
 * Creates a null descriptor, a kernel code descriptor, and a kernel data descriptor.
 * Then loads the GDT and updates the CPU segment registers to use it.
 */
void gdt_init(void);