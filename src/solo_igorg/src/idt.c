#include <idt.h>
#include <libc/stdint.h>

/*
 * IDT entry for 32-bit protected mode.
 * Each entry tells CPU where interrupt handler is located,
 * which code segment selector should be used, and which gate flags apply.
 */
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

/*
 * IDTR pointer structure.
 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/*
 * Assembly function implemented in idt.asm.
 */
extern void idt_load(uint32_t idt_ptr_address);

/*
 * Assembly ISR stubs implemented in isr.asm.
 */
extern void isr0(void);
extern void isr3(void);
extern void isr4(void);

#define IDT_ENTRIES 256
#define KERNEL_CODE_SELECTOR 0x08
#define IDT_INTERRUPT_GATE 0x8E

static struct idt_entry idt_entries[IDT_ENTRIES];
static struct idt_ptr idt_pointer;

static void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t flags)
{
    idt_entries[vector].base_low = (uint16_t)(handler & 0xFFFF);
    idt_entries[vector].selector = selector;
    idt_entries[vector].zero = 0;
    idt_entries[vector].flags = flags;
    idt_entries[vector].base_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

void idt_initialize(void)
{
    idt_pointer.limit = (uint16_t)(sizeof(idt_entries) - 1);
    idt_pointer.base = (uint32_t)&idt_entries;

    /*
     * Clear all IDT entries.
     */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate((uint8_t)i, 0, 0, 0);
    }

    /*
     * Register three software interrupt handlers:
     * 0x00: Divide-by-zero vector
     * 0x03: Breakpoint vector
     * 0x04: Overflow vector
     */
    idt_set_gate(0x00, (uint32_t)isr0, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x03, (uint32_t)isr3, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);
    idt_set_gate(0x04, (uint32_t)isr4, KERNEL_CODE_SELECTOR, IDT_INTERRUPT_GATE);

    idt_load((uint32_t)&idt_pointer);
}
