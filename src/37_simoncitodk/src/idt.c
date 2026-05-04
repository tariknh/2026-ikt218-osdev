#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "ports.h"
#include "terminal.h"
#include <libc/stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEY_RELEASED_MASK 0x80

static struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

static volatile char keyboard_last_key = 0;

static const char scancode_to_ascii[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0,  'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0,  '*',
    0,  ' ', 0
};

extern void idt_flush(void);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

static void idt_set_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt[number].base_low = base & 0xFFFF;
    idt[number].base_high = (base >> 16) & 0xFFFF;

    idt[number].selector = selector;
    idt[number].zero = 0;
    idt[number].flags = flags;
}

char keyboard_get_last_key(void)
{
    return keyboard_last_key;
}

static void keyboard_handle_scancode(uint8_t scancode)
{
    if ((scancode & KEY_RELEASED_MASK) != 0) {
        return;
    }

    char character = scancode_to_ascii[scancode];

    if (character != 0) {
        keyboard_last_key = character;
    }
}

void idt_init(void)
{
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    idt_set_gate(0, (uint32_t)isr0, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(1, (uint32_t)isr1, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(2, (uint32_t)isr2, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);

    idt_set_gate(32, (uint32_t)irq0, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(33, (uint32_t)irq1, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(34, (uint32_t)irq2, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(35, (uint32_t)irq3, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(36, (uint32_t)irq4, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(37, (uint32_t)irq5, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(38, (uint32_t)irq6, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(39, (uint32_t)irq7, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(40, (uint32_t)irq8, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(41, (uint32_t)irq9, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(42, (uint32_t)irq10, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(43, (uint32_t)irq11, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(44, (uint32_t)irq12, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(45, (uint32_t)irq13, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(46, (uint32_t)irq14, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);
    idt_set_gate(47, (uint32_t)irq15, KERNEL_CODE_SEGMENT, IDT_INTERRUPT_GATE);

    idt_flush();
}

void isr_handler(uint32_t interrupt_number)
{
    terminal_write("Interrupt triggered: ");

    if (interrupt_number == 0) {
        terminal_write("0\n");
    } else if (interrupt_number == 1) {
        terminal_write("1\n");
    } else if (interrupt_number == 2) {
        terminal_write("2\n");
    } else {
        terminal_write("unknown\n");
    }
}

void irq_handler(uint32_t irq_number)
{
    if (irq_number == 0) {
        pit_handle_tick();
    } else if (irq_number == 1) {
        uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
        keyboard_handle_scancode(scancode);
    } else {
        terminal_write("IRQ triggered\n");
    }

    pic_send_eoi((uint8_t)irq_number);
}
