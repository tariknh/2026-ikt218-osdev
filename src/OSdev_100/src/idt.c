#include "../include/idt.h"

static idt_entry_t idt_table[IDT_MAX_DESCRIPTORS];
static idtr_t      idt_descriptor;

extern void* isr_stub_table[];



static void set_idt_entry(uint8_t vector, void* handler, uint8_t flags) {
    uint32_t addr      = (uint32_t)handler;
    idt_entry_t* entry = &idt_table[vector];

    // Split handler address across low/high fields
    *entry = (idt_entry_t){
        .isr_low    = addr & 0xFFFF,
        .isr_high   = addr >> 16,
        .kernel_cs  = GDT_OFFSET_KERNEL_CODE,
        .attributes = flags,
        .reserved   = 0,
    };
}

void idt_init(void) {
    // Point IDTR at our table
    idt_descriptor.limit = (sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS) - 1;
    idt_descriptor.base  = (uint32_t)&idt_table;

    // Wire up the first 32 CPU exception handlers
    for (uint8_t i = 0; i < 48; i++) {
        set_idt_entry(i, isr_stub_table[i], 0x8E);
    }

    // Load the IDT register
    __asm__ volatile ("lidt %0" :: "m"(idt_descriptor));
}