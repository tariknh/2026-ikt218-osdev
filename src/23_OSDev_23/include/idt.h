#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* One IDT entry (gate descriptor) — 8 bytes */
struct idt_entry {
    uint16_t base_lo;   /* Lower 16 bits of the ISR address        */
    uint16_t sel;       /* Kernel code segment selector (0x08)      */
    uint8_t  always0;   /* Must always be zero                      */
    uint8_t  flags;     /* Type + DPL + Present bits                */
    uint16_t base_hi;   /* Upper 16 bits of the ISR address         */
} __attribute__((packed));

/* IDT pointer loaded by the lidt instruction */
struct idt_ptr {
    uint16_t limit;     /* Size of the IDT - 1                      */
    uint32_t base;      /* Linear address of the IDT                */
} __attribute__((packed));

/* Set one gate in the IDT */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

/* Zero, fill minimal entries, and load the IDT */
void idt_install(void);

#endif /* IDT_H */
