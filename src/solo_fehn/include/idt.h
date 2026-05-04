/*
 * idt.h - Interrupt Descriptor Table
 *
 * The IDT is the i386 equivalent of the GDT but for interrupts. It holds
 * 256 entries (one per interrupt vector). When the CPU receives interrupt
 * N, it looks up entry N in the IDT, finds the address of a handler, and
 * jumps there.
 *
 * Entry layout (8 bytes):
 *   bits  0..15  base_low      low 16 bits of handler address
 *   bits 16..31  selector      code segment selector (we always use 0x08)
 *   bits 32..39  zero          reserved, must be zero
 *   bits 40..47  flags         type, ring, present-bit
 *   bits 48..63  base_high     high 16 bits of handler address
 *
 * The flags byte 0x8E means "32-bit interrupt gate, ring 0, present".
 */

#pragma once
#include <libc/stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Build the IDT, register all CPU exception handlers (ISR0..31), and load
   it with lidt. After this returns, the CPU knows where to look on every
   interrupt. */
void idt_install(void);

/* Fill in one IDT entry. Used by both isr.c and irq.c. */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
