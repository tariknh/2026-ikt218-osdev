#pragma once
#include "interrupts.h"

struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes) {
    struct idt_gate a;
    a.offset1 = offset;
    a.offset2 = (offset >> 16);
    a.selector = selector;
    attributes = attributes;

    return a;
}
