/*
 * gdt.c - Global Descriptor Table implementation
 *
 * Builds a 3-entry GDT in memory and asks the CPU to load it via the
 * external assembly routine gdt_flush() (defined in gdt.asm).
 *
 * Layout of one GDT entry on i386 (8 bytes total):
 *
 *   bits   field        meaning
 *   0-15   limit_low    low 16 bits of segment limit
 *   16-31  base_low     low 16 bits of segment base address
 *   32-39  base_mid     middle 8 bits of base
 *   40-47  access       type & ring & present-bit flags
 *   48-51  limit_high   high 4 bits of segment limit
 *   52-55  granularity  size & granularity flags
 *   56-63  base_high    high 8 bits of base
 */

#include <gdt.h>
#include <libc/stdint.h>

#define GDT_ENTRIES 3

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;   /* low 4 bits = high 4 bits of limit, top 4 bits = flags */
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;         /* size of the GDT in bytes - 1 */
    uint32_t base;          /* linear address of the first entry */
} __attribute__((packed));

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtp;

/* Implemented in gdt.asm. Loads our GDT and reloads the segment registers. */
extern void gdt_flush(uint32_t gdt_ptr_addr);

static void gdt_set_gate(int n,
                         uint32_t base,
                         uint32_t limit,
                         uint8_t  access,
                         uint8_t  gran)
{
    gdt[n].base_low    = (uint16_t)(base & 0xFFFF);
    gdt[n].base_mid    = (uint8_t)((base >> 16) & 0xFF);
    gdt[n].base_high   = (uint8_t)((base >> 24) & 0xFF);

    gdt[n].limit_low   = (uint16_t)(limit & 0xFFFF);
    /* The high 4 bits of the limit live in the low nibble of granularity, */
    /* and the granularity/size flags live in the high nibble.             */
    gdt[n].granularity = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));

    gdt[n].access      = access;
}

void gdt_install(void)
{
    /* Tell the CPU how large our GDT is and where it lives in memory. */
    gdtp.limit = (uint16_t)(sizeof(struct gdt_entry) * GDT_ENTRIES - 1);
    gdtp.base  = (uint32_t)&gdt;

    /* Index 0: NULL descriptor. Required to be all-zero. */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Index 1: Code (Text) segment.
       base=0, limit=0xFFFFFFFF (full 4 GiB),
       access=0x9A = present | ring0 | code | readable | not-yet-accessed
       gran=0xCF  = page-granular (4 KiB) | 32-bit | limit high nibble = 0xF */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Index 2: Data segment.
       Same base/limit/granularity as code, but access=0x92 (writable instead
       of executable). */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Hand the GDT pointer to the assembly routine so it can run lgdt. */
    gdt_flush((uint32_t)&gdtp);
}
