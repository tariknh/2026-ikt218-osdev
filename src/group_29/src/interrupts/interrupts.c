#include "interrupts.h"

struct idt_gate create_idt_gate(uint32_t offset, uint16_t selector, uint8_t attributes) {
    struct idt_gate a;
    a.low_offset = (offset);
    a.hi_offset = (offset >> 16);
    a.selector = selector;
    a.attributes = attributes;
    a.RESERVED_DO_NOT_USE = 0;

    return a;
}
