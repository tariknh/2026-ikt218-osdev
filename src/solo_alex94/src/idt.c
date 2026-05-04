#include <libc/stdint.h>
#include "idt.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

/* Table of ISR stub addresses generated in interrupts.asm */
extern void* isr_stub_table[IDT_ENTRIES];

void idt_set_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[number].base_low = (uint16_t)(base & 0xFFFF);
    idt[number].selector = selector;
    idt[number].zero = 0;
    idt[number].flags = flags;
    idt[number].base_high = (uint16_t)((base >> 16) & 0xFFFF);
}

void idt_init(void) {
    uint32_t i;

    for (i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate((uint8_t)i, (uint32_t)isr_stub_table[i], KERNEL_CODE_SELECTOR, 0x8E);
    }

    idt_descriptor.limit = (uint16_t)(sizeof(idt) - 1);
    idt_descriptor.base = (uint32_t)&idt;

    __asm__ volatile("lidt %0" : : "m"(idt_descriptor));
}
