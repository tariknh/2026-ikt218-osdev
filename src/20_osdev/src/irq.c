#include <libc/stdint.h>
#include "irq.h"
#include "idt.h"

extern void irq0(void);
extern void irq1(void);

static volatile uint16_t *video = (volatile uint16_t *)0xB8000;

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void print_at(const char *text, int row)
{
    int offset = row * 80;

    for (int i = 0; text[i] != '\0'; i++)
    {
        video[offset + i] = (uint16_t)text[i] | (uint16_t)0x0F00;
    }
}

static void pic_remap(void)
{
    unsigned char a1;
    unsigned char a2;

    a1 = inb(0x21);
    a2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, a1);
    outb(0xA1, a2);
}

static void pic_send_eoi(unsigned char irq)
{
    if (irq >= 8)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}

void irq0_handler(void)
{
    print_at("IRQ 0 timer triggered", 5);
    pic_send_eoi(0);
}

static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, '*', 0, ' '};

void irq1_handler(void)
{
    uint8_t scancode = inb(0x60);

    if (scancode < sizeof(scancode_to_ascii))
    {
        char c = scancode_to_ascii[scancode];

        if (c)
        {
            static int pos = 0;
            video[80 * 8 + pos++] = (uint16_t)c | 0x0F00;
        }
    }

    pic_send_eoi(1);
}

void irq_init(void)
{
    pic_remap();

    idt_set_gate(32, (uint32_t)irq0);
    idt_set_gate(33, (uint32_t)irq1);

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    __asm__ volatile("sti");
}