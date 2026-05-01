#include <kernel/idt.h>
#include <kernel/gdt.h>
#include <kernel/interrupt.h>
#include <libc/stdbool.h>

static struct IdtEntry idt[IDT_ENTRIES];
static struct Idtr idtr;

void IdtSetDescriptor(uint8_t vector, uint32_t interrupt, uint8_t flags) {
    struct IdtEntry* descriptor = &idt[vector];

    descriptor->interrupt_low  = (uint32_t)interrupt & 0xFFFF;
    descriptor->kernel_cs      = GDT_CODE_SELECTOR;
    descriptor->attributes     = flags;
    descriptor->interrupt_high = (uint32_t)interrupt >> 16;
    descriptor->reserved       = 0;
}

extern void* isr_stub_table[];
extern void* irq_stub_table[];

void IdtInitialize(void) {
    idtr.base = (uint32_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(struct IdtEntry) * IDT_ENTRIES - 1;

    for (uint8_t iNum = 0; iNum < 32; iNum++) {
        IdtSetDescriptor(iNum, (uint32_t)isr_stub_table[iNum], 0x8E);
    }

    for (uint8_t iNum = 32; iNum < 48; iNum++) {
        IdtSetDescriptor(iNum, (uint32_t)irq_stub_table[iNum - 32], 0x8E);
    }

    PicRemap();

    __asm__ volatile ("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");
}

