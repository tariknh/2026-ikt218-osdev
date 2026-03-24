#include "interrupts.h"

void load_idt(struct idt_pointer idt_pointer) {
    __asm__ __volatile__ (".intel_syntax noprefix");
    __asm__ __volatile__ ("lidt %0" : : "m" idt_pointer);
}

struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes)
{
    struct idt_gate a;
    a.low_offset = (offset);
    a.hi_offset = (offset >> 16);
    a.selector = selector;
    a.attributes = attributes;
    a.RESERVED_DO_NOT_USE = 0;

    return a;
}

uint8_t create_idt_attributes(bool present, int8_t ring, uint8_t type) {
    uint8_t a = 0;
    if (present) { a = 0b10000000; }
    else if (!present) { a = 0; }
    a = a | (ring << 5);
    a = a | type;
    return a;
}