#include "irq.h"
#include "idt.h"
#include "terminal.h"
#include "libc/system.h"   // outb/inb
#include "libc/stdio.h"    // printf
#include "games/snake.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

// IRQ0 will be mapped to INT 0x20 (32)
// IRQ1 will be mapped to INT 0x21 (33)
#define IRQ0_VECTOR 32
#define IRQ1_VECTOR 33

extern void irq0_stub(void);
extern void irq1_stub(void);

volatile uint32_t timer_ticks = 0;

static void pic_remap(void) {
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    // start init
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    // vector offsets
    outb(PIC1_DATA, 0x20); // master offset 0x20
    outb(PIC2_DATA, 0x28); // slave  offset 0x28

    // tell master/slave wiring
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // restore masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);

    // unmask IRQ0 (timer) and IRQ1 (keyboard), mask everything else
    outb(PIC1_DATA, 0xFC); // 11111100
    outb(PIC2_DATA, 0xFF); // mask all on slave
}

static char scancode_to_ascii(uint8_t scancode) {
    switch (scancode) {
        case 0x11: return 'w';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        default:   return 0;
    }
}

void irq0_handler(void) {
    timer_ticks++;
    outb(PIC1_CMD, PIC_EOI);
}

void irq1_handler(void) {
    uint8_t scancode = inb(0x60);

    // ignore key release events (high bit set)
    if ((scancode & 0x80) == 0) {
        char key = scancode_to_ascii(scancode);
        if (key != 0) {
            snake_handle_key(key);
        }
    }

    outb(PIC1_CMD, PIC_EOI);
}

void irq_install(void) {
    pic_remap();

    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));

    // 0x8E = present | ring0 | 32-bit interrupt gate
    idt_set_gate(IRQ0_VECTOR, (uint32_t)irq0_stub, cs, 0x8E);
    idt_set_gate(IRQ1_VECTOR, (uint32_t)irq1_stub, cs, 0x8E);
}