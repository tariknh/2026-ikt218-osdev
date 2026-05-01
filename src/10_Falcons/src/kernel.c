#include <kernel/piano.h>
#include <kernel/terminal.h>
#include <stdint.h>
#include <kernel/memory.h>
#include <kernel/pit.h>

extern uint32_t end;

#include <libc/stddef.h>
#include <libc/stdint.h>
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include <song/song.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

static uint8_t vga_entry_color(uint8_t foreground, uint8_t background) {
    return foreground | background << 4;
}

static uint16_t vga_entry(unsigned char character, uint8_t color) {
    return (uint16_t) character | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(15, 0);
    terminal_buffer = (uint16_t*) VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        return;
    }

    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = vga_entry(c, terminal_color);

    terminal_column++;

    if (terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row == VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_write(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void kernel_main(void) {
    gdt_init();
    idt_init();
    isr_install();
    irq_install();

    terminal_initialize();
    keyboard_init();


    /* Assignment 4: memory manager, paging, malloc/free and PIT. */
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    void *mem1 = malloc(12345);
    void *mem2 = malloc(54321);
    void *mem3 = malloc(13331);
    printf("[MEM] malloc test: mem1=0x%x mem2=0x%x mem3=0x%x\n", (uint32_t)mem1, (uint32_t)mem2, (uint32_t)mem3);
    free(mem2);
    void *mem4 = malloc(1000);
    printf("[MEM] free/reuse test: mem4=0x%x\n", (uint32_t)mem4);

    init_pit();
    piano_init();
    __asm__ volatile ("sti");
    printf("[PIT] Testing busy sleep 250 ms...\n");
    sleep_busy(250);
    printf("[PIT] Busy sleep OK. Testing interrupt sleep 250 ms...\n");
    sleep_interrupt(250);
    printf("[PIT] Interrupt sleep OK.\n");

    printf("[MUSIC] Starting PC speaker music test...\n");
    play_music();
    printf("[MUSIC] Music test completed.\n");
    terminal_write("Keyboard ready. Type something:\n");

    asm volatile("sti");

    while (1) {
        asm volatile("hlt");
    }
}

/* Simple kernel printf implementation for Assignment 4.
   Supports: %s, %c, %d, %u, %x, %p and %% */
typedef __builtin_va_list k_va_list;
#define k_va_start(ap, last) __builtin_va_start(ap, last)
#define k_va_arg(ap, type) __builtin_va_arg(ap, type)
#define k_va_end(ap) __builtin_va_end(ap)

static void printf_put_unsigned(unsigned int value)
{
    char buffer[16];
    int i = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

static void printf_put_signed(int value)
{
    if (value < 0) {
        terminal_putchar('-');
        value = -value;
    }

    printf_put_unsigned((unsigned int)value);
}

static void printf_put_hex(unsigned int value)
{
    const char *hex = "0123456789ABCDEF";
    int started = 0;

    for (int i = 28; i >= 0; i -= 4) {
        unsigned int digit = (value >> i) & 0xF;

        if (digit != 0 || started || i == 0) {
            terminal_putchar(hex[digit]);
            started = 1;
        }
    }
}

void printf(const char *format, ...)
{
    k_va_list args;
    k_va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            terminal_putchar(format[i]);
            continue;
        }

        i++;
        char specifier = format[i];

        if (specifier == '\0') {
            break;
        }

        switch (specifier) {
            case 's': {
                const char *str = k_va_arg(args, const char *);
                if (!str) {
                    str = "(null)";
                }
                while (*str) {
                    terminal_putchar(*str++);
                }
                break;
            }

            case 'c': {
                char c = (char)k_va_arg(args, int);
                terminal_putchar(c);
                break;
            }

            case 'd': {
                int value = k_va_arg(args, int);
                printf_put_signed(value);
                break;
            }

            case 'u': {
                unsigned int value = k_va_arg(args, unsigned int);
                printf_put_unsigned(value);
                break;
            }

            case 'x': {
                unsigned int value = k_va_arg(args, unsigned int);
                printf_put_hex(value);
                break;
            }

            case 'p': {
                unsigned int value = (unsigned int)k_va_arg(args, void *);
                terminal_putchar('0');
                terminal_putchar('x');
                printf_put_hex(value);
                break;
            }

            case '%': {
                terminal_putchar('%');
                break;
            }

            default: {
                terminal_putchar('%');
                terminal_putchar(specifier);
                break;
            }
        }
    }

    k_va_end(args);
}
