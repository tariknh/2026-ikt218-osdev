#include "irq.h"
#include "io.h"
#include <stddef.h>

#define MASTER_CMD   0x20
#define MASTER_DATA  0x21
#define SLAVE_CMD    0xA0
#define SLAVE_DATA   0xA1
#define PIC_EOI      0x20
#define ICW1_INIT    0x10
#define ICW1_ICW4    0x01
#define ICW4_8086    0x01

static irq_handler_t irq_handlers[16] = { NULL };

extern void terminal_putchar(char c);
extern void terminal_write(const char* str);

static const char scancode_to_ascii[128] = {
    0,   0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static void keyboard_handler(registers_t* regs)
{
    (void)regs;
    uint8_t scancode = inb(0x60);
    if (scancode & 0x80) return;
    char c = scancode_to_ascii[scancode];
    if (c != 0) terminal_putchar(c);
}

static void pic_remap(int offset1, int offset2)
{
    uint8_t mask_master = inb(MASTER_DATA);
    uint8_t mask_slave  = inb(SLAVE_DATA);

    outb(MASTER_CMD,  ICW1_INIT | ICW1_ICW4); io_wait();
    outb(SLAVE_CMD,   ICW1_INIT | ICW1_ICW4); io_wait();
    outb(MASTER_DATA, (uint8_t)offset1);       io_wait();
    outb(SLAVE_DATA,  (uint8_t)offset2);       io_wait();
    outb(MASTER_DATA, 0x04);                   io_wait();
    outb(SLAVE_DATA,  0x02);                   io_wait();
    outb(MASTER_DATA, ICW4_8086);              io_wait();
    outb(SLAVE_DATA,  ICW4_8086);              io_wait();

    outb(MASTER_DATA, mask_master);
    outb(SLAVE_DATA,  mask_slave);
}

void irq_init(void)
{
    pic_remap(0x20, 0x28);
    outb(MASTER_DATA, 0x00);
    outb(SLAVE_DATA,  0x00);
    irq_register_handler(1, keyboard_handler);
}

void irq_register_handler(int irq, irq_handler_t handler)
{
    if (irq >= 0 && irq < 16)
        irq_handlers[irq] = handler;
}

void irq_handler(registers_t* regs)
{
    int irq = (int)regs->int_no - 32;
    if (irq >= 8) outb(SLAVE_CMD, PIC_EOI);
    outb(MASTER_CMD, PIC_EOI);
    if (irq >= 0 && irq < 16 && irq_handlers[irq] != NULL)
        irq_handlers[irq](regs);
}
