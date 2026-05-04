#include "idt.h"
#include "isr.h"
#include "irq.h"

#define IDT_ENTRY_COUNT 256

static struct idt_entry idt_entries[IDT_ENTRY_COUNT];
static struct idt_ptr idt_pointer;

//fills one IDT entry with the address of its handler function
static void idt_set_entry(uint8_t index, void* handler, uint8_t type_attr) {
    idt_entries[index].offset_low = (uint32_t)handler & 0xFFFF;
    idt_entries[index].selector = 0x08; //kernel code segment from GDT
    idt_entries[index].zero = 0;
    idt_entries[index].type_attr = type_attr;
    idt_entries[index].offset_high = (uint32_t)handler >> 16;
}

void init_idt(void) {
    idt_pointer.limit = (sizeof(struct idt_entry) * IDT_ENTRY_COUNT) - 1;
    idt_pointer.base = (uint32_t)&idt_entries;

    //register all 32 CPU exception handlers
    idt_set_entry(0, isr0, 0x8E);
    idt_set_entry(1, isr1, 0x8E);
    idt_set_entry(2, isr2, 0x8E);
    idt_set_entry(3, isr3, 0x8E);
    idt_set_entry(4, isr4, 0x8E);
    idt_set_entry(5, isr5, 0x8E);
    idt_set_entry(6, isr6, 0x8E);
    idt_set_entry(7, isr7, 0x8E);
    idt_set_entry(8, isr8, 0x8E);
    idt_set_entry(9, isr9, 0x8E);
    idt_set_entry(10, isr10, 0x8E);
    idt_set_entry(11, isr11, 0x8E);
    idt_set_entry(12, isr12, 0x8E);
    idt_set_entry(13, isr13, 0x8E);
    idt_set_entry(14, isr14, 0x8E);
    idt_set_entry(15, isr15, 0x8E);
    idt_set_entry(16, isr16, 0x8E);
    idt_set_entry(17, isr17, 0x8E);
    idt_set_entry(18, isr18, 0x8E);
    idt_set_entry(19, isr19, 0x8E);
    idt_set_entry(20, isr20, 0x8E);
    idt_set_entry(21, isr21, 0x8E);
    idt_set_entry(22, isr22, 0x8E);
    idt_set_entry(23, isr23, 0x8E);
    idt_set_entry(24, isr24, 0x8E);
    idt_set_entry(25, isr25, 0x8E);
    idt_set_entry(26, isr26, 0x8E);
    idt_set_entry(27, isr27, 0x8E);
    idt_set_entry(28, isr28, 0x8E);
    idt_set_entry(29, isr29, 0x8E);
    idt_set_entry(30, isr30, 0x8E);
    idt_set_entry(31, isr31, 0x8E);

    //register IRQ0-15 handlers (mapped to vectors 0x20-0x2F after PIC remap)
    idt_set_entry(32, irq0, 0x8E);
    idt_set_entry(33, irq1, 0x8E);
    idt_set_entry(34, irq2, 0x8E);
    idt_set_entry(35, irq3, 0x8E);
    idt_set_entry(36, irq4, 0x8E);
    idt_set_entry(37, irq5, 0x8E);
    idt_set_entry(38, irq6, 0x8E);
    idt_set_entry(39, irq7, 0x8E);
    idt_set_entry(40, irq8, 0x8E);
    idt_set_entry(41, irq9, 0x8E);
    idt_set_entry(42, irq10, 0x8E);
    idt_set_entry(43, irq11, 0x8E);
    idt_set_entry(44, irq12, 0x8E);
    idt_set_entry(45, irq13, 0x8E);
    idt_set_entry(46, irq14, 0x8E);
    idt_set_entry(47, irq15, 0x8E);

    //load the IDT and enable interrupts
    __asm__ volatile ("lidt %0" : : "m"(idt_pointer));
}