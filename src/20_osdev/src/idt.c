#include <libc/stdint.h>
#include "idt.h"

#define IDT_ENTRIES 256
#define CODE_SEGMENT 0x08

struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_descriptor;

static void idt_set_entry(int index, uint32_t handler, uint16_t selector, uint8_t type_attr)
{
    idt[index].offset_low = handler & 0xFFFF;
    idt[index].selector = selector;
    idt[index].zero = 0;
    idt[index].type_attr = type_attr;
    idt[index].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_set_gate(int index, uint32_t handler)
{
    idt_set_entry(index, handler, CODE_SEGMENT, 0x8E);
}

void idt_init(void)
{
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (uint32_t)&idt;

    for (int i = 0; i < IDT_ENTRIES; i++)
    {
        idt_set_entry(i, 0, CODE_SEGMENT, 0x8E);
    }

    __asm__ volatile("lidt %0" : : "m"(idt_descriptor));
}