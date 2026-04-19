#include <libc/stdint.h>
#include <keyboard.h>
#include <pic.h>
#include <pit.h>
#include <input.h>

extern void terminal_write(const char *str);

static int shift_held = 0;

void irq_handler(uint32_t vector)
{
    uint32_t irq_number = vector - 32;

    if (irq_number == 0) {
        pit_handler();  /* advance PIT tick counter */
    } else if (irq_number == 1) {
        unsigned char scancode = inb(0x60);

        /* Track shift key state */
        if (scancode == 0x2A || scancode == 0x36) {      /* shift pressed  */
            shift_held = 1;
        } else if (scancode == 0xAA || scancode == 0xB6) { /* shift released */
            shift_held = 0;
        } else if ((scancode & 0x80) == 0) {             /* key press */
            uint8_t ascii;

            if (shift_held)
                ascii = scancode2ascii_shift[scancode];
            else
                ascii = scancode2ascii[scancode];

            /* Map Enter scancode (0x1C) to newline */
            if (scancode == 0x1C) ascii = '\n';
            /* Map Backspace scancode (0x0E) */
            if (scancode == 0x0E) ascii = '\b';
            /* Map Escape scancode (0x01) */
            if (scancode == 0x01) ascii = 0x1B;

            if (ascii != 0)
                input_put_char((char)ascii);
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
