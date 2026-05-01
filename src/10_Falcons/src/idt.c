#include <libc/stdint.h>
#include "idt.h"

struct idt_entry idt_entries[256];
struct idt_ptr idt_pointer;

extern void idt_load(uint32_t);

void idt_set_gate(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[index].base_low = base & 0xFFFF;
    idt_entries[index].base_high = (base >> 16) & 0xFFFF;

    idt_entries[index].selector = selector;
    idt_entries[index].always_zero = 0;
    idt_entries[index].flags = flags;
}

void idt_init(void) {
    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base = (uint32_t)&idt_entries;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idt_load((uint32_t)&idt_pointer);
}