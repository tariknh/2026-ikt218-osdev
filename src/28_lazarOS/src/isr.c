#include <libc/stdint.h>
#include <keyboard.h>
#include <pic.h>

extern void terminal_write(const char *str);

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void irq_handler(uint32_t vector)
{
    uint32_t irq_number = vector - 32;

    if (irq_number == 0) {
        static int tick_count = 0;
        tick_count++;

        if (tick_count % 100 == 0)
            terminal_write("TIMER\n");
    } else if (irq_number == 1) {
        unsigned char scancode = inb(0x60);

        if ((scancode & 0x80) == 0) {
            uint8_t ascii = scancode2ascii[scancode];
            if (ascii != 0) {
                char ch[2];
                ch[0] = (char)ascii;
                ch[1] = '\0';
                terminal_write(ch);
            }
        }
    }

    if (irq_number >= 8)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);
}

void exception_handler(uint32_t vector)
{
    if (vector == 0)
        terminal_write("INT 0\n");
    if (vector == 3)
        terminal_write("INT 3\n");
    if (vector == 6)
        terminal_write("INT 6\n");
}
