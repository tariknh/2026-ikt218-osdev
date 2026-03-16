#include "gdt.h"

/*
 * gdt.c - Global Descriptor Table (GDT) initialisation
 *
 * The GDT tells the CPU how the address space is segmented.
 * In a flat 32-bit kernel we use three descriptors:
 *   0 - Null descriptor     (required; CPU fault if this is selected)
 *   1 - Kernel code segment (ring 0, executable, 4 GB flat)
 *   2 - Kernel data segment (ring 0, writable,   4 GB flat)
 *
 * After filling the table we call gdt_flush() (in gdt.asm) which
 * executes the lgdt instruction and reloads all segment registers.
 */

/* The three GDT entries */
static struct gdt_entry gdt[GDT_ENTRY_COUNT];

/* The GDT pointer loaded by lgdt */
static struct gdt_ptr gp;

/*
 * gdt_flush - defined in src/arch/i386/gdt.asm
 *
 * Loads the GDT via lgdt and then performs a far jump to flush
 * the CPU pipeline and reload CS.  All other segment registers
 * (DS, ES, FS, GS, SS) are reloaded with the data selector.
 */
extern void gdt_flush(struct gdt_ptr* ptr);

/*
 * gdt_set_entry - fill one GDT slot
 *
 * @num    index into the gdt[] array
 * @base   32-bit linear base address of the segment
 * @limit  20-bit limit (in pages if G=1, in bytes if G=0)
 * @access access byte: present, DPL, S, type bits
 * @flags  upper nibble of the granularity byte:
 *           bit 7 (G)  - granularity: 1 = 4 KB pages, 0 = bytes
 *           bit 6 (DB) - default/big: 1 = 32-bit, 0 = 16-bit
 *           bit 5 (L)  - long mode:  0 for 32-bit segments
 *           bit 4      - reserved, always 0
 */
static void gdt_set_entry(int num, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags)
{
    gdt[num].base_low    = (uint16_t)(base & 0xFFFF);
    gdt[num].base_mid    = (uint8_t)((base >> 16) & 0xFF);
    gdt[num].base_high   = (uint8_t)((base >> 24) & 0xFF);

    gdt[num].limit_low   = (uint16_t)(limit & 0xFFFF);

    /*
     * granularity byte:
     *   upper nibble = flags (G, DB, L, reserved)
     *   lower nibble = bits 19-16 of the 20-bit limit
     */
    gdt[num].granularity = (flags & 0xF0) | ((limit >> 16) & 0x0F);

    gdt[num].access = access;
}

/*
 * gdt_init - set up and load the GDT
 *
 * Access byte reference (bit fields):
 *   Bit 7 (P)   - Present: must be 1 for valid descriptors
 *   Bits 6-5 (DPL) - Descriptor Privilege Level (0 = ring 0 = kernel)
 *   Bit 4 (S)   - Descriptor type: 1 = code/data, 0 = system
 *   Bit 3 (E)   - Executable: 1 = code segment, 0 = data segment
 *   Bit 2 (DC)  - Direction/Conforming (0 for flat kernel segments)
 *   Bit 1 (RW)  - Readable (code) / Writable (data): 1 = yes
 *   Bit 0 (A)   - Accessed: CPU sets this; we leave it 0
 *
 * Flags nibble (upper nibble of granularity byte):
 *   Bit 7 (G)   - 1: limit in 4 KB units -> max segment = 4 GB
 *   Bit 6 (DB)  - 1: 32-bit default operand / stack size
 *   Bit 5 (L)   - 0: not a 64-bit code segment
 *   Bit 4       - 0: reserved
 */
void gdt_init(void)
{
    /* Point the GDT descriptor at our table */
    gp.limit = (uint16_t)(sizeof(struct gdt_entry) * GDT_ENTRY_COUNT - 1);
    gp.base  = (uint32_t)&gdt;

    /* --- Entry 0: Null descriptor --- */
    /* The CPU requires the first entry to be all zeros. */
    gdt_set_entry(0, 0, 0, 0x00, 0x00);

    /* --- Entry 1: Kernel code segment --- */
    /* Base=0, Limit=0xFFFFF (4 GB with 4 KB granularity)
     * Access = 0x9A  =>  P=1, DPL=00, S=1, E=1, DC=0, R=1, A=0
     * Flags  = 0xC0  =>  G=1, DB=1, L=0, reserved=0           */
    gdt_set_entry(1, 0x00000000, 0xFFFFF, 0x9A, 0xC0);

    /* --- Entry 2: Kernel data segment --- */
    /* Base=0, Limit=0xFFFFF (4 GB with 4 KB granularity)
     * Access = 0x92  =>  P=1, DPL=00, S=1, E=0, DC=0, W=1, A=0
     * Flags  = 0xC0  =>  G=1, DB=1, L=0, reserved=0           */
    gdt_set_entry(2, 0x00000000, 0xFFFFF, 0x92, 0xC0);

    /* Load the new GDT and flush the segment registers */
    gdt_flush(&gp);
}
