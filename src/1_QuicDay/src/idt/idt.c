#include "idt.h"
#include "isr.h"

__attribute__((aligned(0x10)))
static idt_entry_t_64 idt[256];

static idtr_t_64 idtr;

extern void idt_load(idtr_t_64* idtr);

static void idt_set_entry(int vector, void (*isr)(void), uint8_t flags) {
    uint64_t addr = (uint64_t)isr;

    idt[vector].isr_low    = (uint16_t)(addr & 0xFFFF);
    idt[vector].kernel_cs  = 0x08;          // kernel code segment
    idt[vector].ist        = 0;             // no IST for now
    idt[vector].attributes = flags;         // e.g. 0x8E = present, ring 0, interrupt gate
    idt[vector].isr_mid    = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[vector].isr_high   = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[vector].reserved   = 0;
}

void idt_init(void) {
    for (int i = 0; i < 256; ++i) {
        idt[i].isr_low    = 0;
        idt[i].kernel_cs  = 0;
        idt[i].ist        = 0;
        idt[i].attributes = 0;
        idt[i].isr_mid    = 0;
        idt[i].isr_high   = 0;
        idt[i].reserved   = 0;
    }

    // Install at least three ISRs
    idt_set_entry(0,  (void*)isr0,  0x8E); // divide-by-zero
    idt_set_entry(1,  (void*)isr1,  0x8E); // debug
    idt_set_entry(14, (void*)isr14, 0x8E); // page fault

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    idt_load(&idtr);
}
