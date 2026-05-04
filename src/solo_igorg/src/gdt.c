#include <gdt.h>
#include <libc/stdint.h>

/*
 * GDT entry is exactly 8 bytes.
 *
 * The fields are split because the x86 descriptor format does not store
 * base and limit as simple continuous values. Instead, parts of the base
 * address, segment limit, access byte, and flags are packed together.
 */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/*
 * GDTR register is loaded with a pointer containing:
 *  - size of GDT minus one
 *  - address of first GDT entry
 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/*
 * This assembly function is implemented in gdt.asm.
 * It executes lgdt and reloads registers.
 */
extern void gdt_flush(uint32_t gdt_ptr_address);

static struct gdt_entry gdt_entries[3];
static struct gdt_ptr gdt_pointer;

static void gdt_set_entry(
    int index,
    uint32_t base,
    uint32_t limit,
    uint8_t access,
    uint8_t granularity
)
{
    gdt_entries[index].base_low = (uint16_t)(base & 0xFFFF);
    gdt_entries[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt_entries[index].base_high = (uint8_t)((base >> 24) & 0xFF);

    gdt_entries[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt_entries[index].granularity = (uint8_t)((limit >> 16) & 0x0F);

    gdt_entries[index].granularity |= (granularity & 0xF0);
    gdt_entries[index].access = access;
}

void gdt_initialize(void)
{
    gdt_pointer.limit = (uint16_t)(sizeof(gdt_entries) - 1);
    gdt_pointer.base = (uint32_t)&gdt_entries;

    /*
     * Entry 0: null descriptor.
     * Selector pointing to this descriptor is invalid.
     */
    gdt_set_entry(0, 0, 0, 0, 0);

    /*
     * Entry 1: kernel code/text.
     *
     * base  = 0x00000000
     * limit = 0xFFFFFFFF through 4 KiB granularity
     *
     * access      = 0x9A
     * granularity = 0xCF
     */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /*
     * Entry 2: kernel data.
     *
     * access      = 0x92
     * granularity = 0xCF
     */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush((uint32_t)&gdt_pointer);
}
