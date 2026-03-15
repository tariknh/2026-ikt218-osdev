#ifndef GDT_H
#define GDT_H

#include <libc/stdint.h>

/*
 * GDT Entry (Segment Descriptor) - 8 bytes
 *
 * Each descriptor describes a memory segment's base address, size (limit),
 * and access permissions.
 *
 *  63       56 55    52 51   48 47     40 39      16 15      0
 * +-----------+--------+-------+---------+----------+---------+
 * | base[31:24]| flags  |lim[19:16]| access | base[23:0]|limit[15:0]|
 * +-----------+--------+-------+---------+----------+---------+
 */
typedef struct {
    uint16_t limit_low;    /* Lower 16 bits of segment limit */
    uint16_t base_low;     /* Lower 16 bits of base address */
    uint8_t  base_middle;  /* Bits 16-23 of base address */
    uint8_t  access;       /* Access byte: present, privilege, type flags */
    uint8_t  granularity;  /* Upper 4 bits of limit + flags (G, D/B, L, AVL) */
    uint8_t  base_high;    /* Upper 8 bits of base address */
} __attribute__((packed)) gdt_entry_t;

/*
 * GDT Descriptor (pointer passed to lgdt instruction) - 6 bytes
 * Contains the size (limit) and address (base) of the GDT.
 */
typedef struct {
    uint16_t limit;  /* Size of GDT in bytes minus 1 */
    uint32_t base;   /* Linear address of the GDT */
} __attribute__((packed)) gdt_descriptor_t;

/* GDT segment selector offsets (index * 8) */
#define GDT_SELECTOR_NULL   0x00  /* NULL descriptor */
#define GDT_SELECTOR_CODE   0x08  /* Kernel code segment */
#define GDT_SELECTOR_DATA   0x10  /* Kernel data segment */

/* Initialise the GDT and load it via lgdt */
void gdt_init(void);

/* Defined in gdt.asm: loads the GDT and reloads segment registers */
extern void gdt_load(gdt_descriptor_t *descriptor);
extern void gdt_reload_segments(void);

#endif /* GDT_H */
