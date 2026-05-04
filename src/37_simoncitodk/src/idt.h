#ifndef IDT_H
#define IDT_H

#include <libc/stdint.h>

#define IDT_ENTRIES 256
#define KERNEL_CODE_SEGMENT 0x08
#define IDT_INTERRUPT_GATE 0x8E

/*
 * One entry in the Interrupt Descriptor Table.
 *
 * In 32-bit protected mode, the CPU uses an interrupt number
 * as an index into the IDT. The selected entry tells the CPU
 * which kernel code segment to use and where the interrupt
 * handler is located in memory.
 */
struct idt_entry {
    uint16_t base_low;     /* Lower 16 bits of the handler address */
    uint16_t selector;     /* Kernel code segment selector from the GDT */
    uint8_t zero;          /* Always set to 0 */
    uint8_t flags;         /* Type and attributes for this IDT entry */
    uint16_t base_high;    /* Upper 16 bits of the handler address */
} __attribute__((packed));

/*
 * Pointer structure loaded by the lidt instruction.
 *
 * This is similar in idea to the GDT pointer used by lgdt:
 * limit = size of the table in bytes minus 1
 * base  = address of the first IDT entry
 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_init(void);
char keyboard_get_last_key(void);

#endif
