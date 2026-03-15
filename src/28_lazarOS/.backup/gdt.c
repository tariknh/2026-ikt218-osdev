#include <gdt.h>

/*
 * The GDT holds 3 descriptors:
 *   [0] NULL descriptor  – required by the CPU; all fields zero
 *   [1] Code descriptor  – kernel code segment (ring 0, executable)
 *   [2] Data descriptor  – kernel data segment (ring 0, writable)
 */
static gdt_entry_t     gdt[3];
static gdt_descriptor_t gdt_descriptor;

/*
 * gdt_set_entry – encode and write one GDT entry.
 *
 * @index     : index into the gdt[] array
 * @base      : 32-bit linear base address of the segment
 * @limit     : 20-bit segment limit (in pages when G=1, in bytes when G=0)
 * @access    : access byte (present, DPL, descriptor type, type flags)
 * @granularity: upper nibble = flags (G, D/B, L, AVL), lower nibble = limit[19:16]
 */
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t granularity)
{
    /* Encode the 32-bit base address across three fields */
    gdt[index].base_low    = (uint16_t)(base & 0xFFFF);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[index].base_high   = (uint8_t)((base >> 24) & 0xFF);

    /* Encode the 20-bit limit: low 16 bits here, high 4 bits in granularity */
    gdt[index].limit_low   = (uint16_t)(limit & 0xFFFF);

    /* Upper 4 bits of granularity byte carry limit[19:16] */
    gdt[index].granularity = (uint8_t)((granularity & 0xF0) | ((limit >> 16) & 0x0F));

    gdt[index].access      = access;
}

/*
 * gdt_init – set up the three required descriptors, fill the GDT descriptor
 *            struct, then call the assembly routine to lgdt + reload segments.
 *
 * Access byte breakdown used below:
 *   Bit 7 (P)   : Segment present (1)
 *   Bits 6-5 (DPL): Descriptor Protection Level – 0 for ring 0
 *   Bit 4 (S)   : Descriptor type: 1 = code/data, 0 = system
 *   Bits 3-0    : Segment type
 *     Code: 1010 = executable + readable
 *     Data: 0010 = writable
 *
 * Granularity byte (upper nibble):
 *   Bit 7 (G)   : Granularity – 1 = 4 KB pages
 *   Bit 6 (D/B) : Default operation size – 1 = 32-bit segment
 *   Bit 5 (L)   : 64-bit code segment flag – 0 (we are 32-bit)
 *   Bit 4 (AVL) : Available for OS use – 0
 */
void gdt_init(void)
{
    /* [0] NULL descriptor – all fields must be zero */
    gdt_set_entry(0, 0, 0, 0x00, 0x00);

    /*
     * [1] Kernel Code Segment
     *   base  = 0x00000000 (flat, starts at address 0)
     *   limit = 0xFFFFF    (with G=1 this covers the full 4 GB)
     *   access= 0x9A       = 1001 1010b
     *   gran  = 0xCF       = 1100 xxxx  (G=1, D/B=1, L=0, AVL=0)
     */
    gdt_set_entry(1, 0x00000000, 0xFFFFF, 0x9A, 0xC0);

    /*
     * [2] Kernel Data Segment
     *   base  = 0x00000000
     *   limit = 0xFFFFF
     *   access= 0x92       = 1001 0010b
     *   gran  = 0xCF       (same flags as code segment)
     */
    gdt_set_entry(2, 0x00000000, 0xFFFFF, 0x92, 0xC0);

    /* Build the 6-byte descriptor the CPU reads via lgdt */
    gdt_descriptor.limit = (uint16_t)(sizeof(gdt) - 1);
    gdt_descriptor.base  = (uint32_t)&gdt;

    /* Execute lgdt and reload all segment registers (defined in gdt.asm) */
    gdt_load(&gdt_descriptor);
    gdt_reload_segments();
}
