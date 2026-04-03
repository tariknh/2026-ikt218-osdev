#include <libc/stdint.h>
#include <keyboard.h>
#include <pic.h>
#include <pit.h>

extern void terminal_write(const char *str);

#define KEYBOARD_BUFFER_SIZE 256

static uint8_t keyboard_scancode_buffer[KEYBOARD_BUFFER_SIZE];
static char keyboard_ascii_buffer[KEYBOARD_BUFFER_SIZE];
static uint16_t keyboard_buffer_write_index = 0;

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void append_uint(char *buf, uint32_t value)
{
    char tmp[11];
    int i = 0;
    int j;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {
        tmp[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    for (j = 0; j < i; j++)
        buf[j] = tmp[i - 1 - j];

    buf[i] = '\0';
}

void irq_handler(uint32_t vector)
{
    uint32_t irq_number = vector - 32;

    if (irq_number == 0) {
        pit_handler();  /* advance PIT tick counter */
    } else if (irq_number == 1) {
        unsigned char scancode = inb(0x60);

        if ((scancode & 0x80) == 0) {
            uint16_t idx = keyboard_buffer_write_index;
            uint8_t ascii = scancode2ascii[scancode];

            keyboard_scancode_buffer[idx] = scancode;
            keyboard_ascii_buffer[idx] = (char)ascii;
            keyboard_buffer_write_index = (uint16_t)((idx + 1) % KEYBOARD_BUFFER_SIZE);

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
