#include "idt.h"
#include <stdint.h>
#include <stddef.h>

/* The IDT: 256 possible interrupt vectors */
struct idt_entry idt[256];
struct idt_ptr   idtp;

/* Defined in multiboot2.asm — loads the IDT pointer with lidt */
extern void _idt_flush(void);

/* Fill a single IDT gate */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    /* OR with 0x60 to allow ring-3 software interrupts (int $0xN from user mode).
       For pure kernel use you can just pass flags directly. */
    idt[num].flags   = flags;
}

/* Zero the IDT, set the pointer, and load it */
void idt_install(void)
{
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;

    /* Clear the entire table to zero (null descriptors) */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    _idt_flush();
}
