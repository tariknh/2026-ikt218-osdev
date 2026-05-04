#include "arch/i386/idt.h"
#include "stdint.h"

#define IDT_ENTRIES 256

/*
* Interrupt Descriptor Table (IDT)
*
* Reference:
* OSDev Wiki - Interrupts Tutorial
* https://wiki.osdev.org/Interrupts_Tutorial
*/

typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t base_high;
} __attribute__((packed)) idt_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_t idt[IDT_ENTRIES];
static idt_ptr_t idt_descriptor;

void disable_interrupts(void) {
    __asm__ volatile ("cli");
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void idt_set_gate(uint8_t index, uint32_t base, uint8_t attributes) {
    idt_t *descriptor = &idt[index];

    descriptor->base_low = (uint16_t) (base & 0xFFFF);
    descriptor->selector = 0x08;
    descriptor->reserved = 0;
    descriptor->attributes = attributes;
    descriptor->base_high = (uint16_t) (base >> 16 & 0xFFFF);
}

void load_idt(void) {
    __asm__ volatile ("lidt %0" : : "m"(idt_descriptor));
}

void init_idt(void) {
    idt_descriptor.limit = (uint16_t) (sizeof(idt) - 1);
    idt_descriptor.base = (uint32_t) &idt;

    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0);
    }

    load_idt();
}
