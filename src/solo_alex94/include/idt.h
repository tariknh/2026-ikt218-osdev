#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

#define IDT_ENTRIES 256
#define KERNEL_CODE_SELECTOR 0x08

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_set_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags);
void idt_init(void);

#endif
