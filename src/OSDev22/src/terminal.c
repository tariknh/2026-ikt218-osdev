#include "terminal.h"
#include "libc/stdarg.h"

/*
 * terminal.c - VGA text-mode terminal driver
 *
 * VGA text mode uses a memory-mapped framebuffer starting at physical
 * address 0xB8000.  Each character cell occupies 2 bytes:
 *   byte 0 - ASCII character code
 *   byte 1 - attribute byte  (bits 7-4 = background colour, bits 3-0 = foreground)
 *
 * The standard text screen is 80 columns × 25 rows = 2 000 cells = 4 000 bytes.
 */

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* Physical address of the VGA text-mode framebuffer */
#define VGA_MEMORY ((uint16_t*)0xB8000)

/* -------------------------------------------------------------------------
 * Module-private state
 * ---------------------------------------------------------------------- */

static size_t   terminal_row;    /* Current cursor row    (0 = top)  */
static size_t   terminal_col;    /* Current cursor column (0 = left) */
static uint8_t  terminal_color;  /* Current attribute byte            */
static uint16_t *terminal_buffer;/* Pointer to VGA framebuffer        */

/* -------------------------------------------------------------------------
 * Helper: pack character + attribute into a 16-bit VGA cell
 * ---------------------------------------------------------------------- */

uint8_t vga_entry_color(vga_color fg, vga_color bg)
{
    return (uint8_t)(fg | (bg << 4));
}

static uint16_t vga_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* -------------------------------------------------------------------------
 * Scroll the terminal up by one row
 *
 * Every row is copied to the row above it; the last row is cleared.
 * ---------------------------------------------------------------------- */
static void terminal_scroll(void)
{
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    }

    /* Blank the newly exposed bottom row */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void terminal_initialize(void)
{
    terminal_row    = 0;
    terminal_col    = 0;
    terminal_color  = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    /* Fill every cell with a space in the current colour to clear the screen */
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putchar(char c)
{
    if (c == '\n') {
        /* Move to the start of the next line */
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    /* Write the character into the framebuffer at the current cursor position */
    terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
        vga_entry((unsigned char)c, terminal_color);

    /* Advance the cursor; wrap to the next line if we hit the right edge */
    if (++terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_writestring(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

/* -------------------------------------------------------------------------
 * printf - minimal formatted output
 *
 * Supported conversion specifiers:
 *   %c   - single character
 *   %s   - null-terminated string
 *   %d   - signed 32-bit decimal integer
 *   %u   - unsigned 32-bit decimal integer
 *   %x   - unsigned 32-bit hexadecimal (lowercase, no leading zeros)
 *   %%   - literal percent sign
 * ---------------------------------------------------------------------- */

/* Write an unsigned integer in the given base (2-16) */
static void print_uint(uint32_t value, uint32_t base)
{
    static const char digits[] = "0123456789abcdef";
    char buf[32];
    int  pos = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    /* Build the digit string in reverse order */
    while (value > 0) {
        buf[pos++] = digits[value % base];
        value /= base;
    }

    /* Print in correct (forward) order */
    while (pos > 0) {
        terminal_putchar(buf[--pos]);
    }
}

void printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            terminal_putchar(fmt[i]);
            continue;
        }

        /* Consume the '%' and look at the next character */
        i++;
        switch (fmt[i]) {
            case 'c':
                terminal_putchar((char)va_arg(args, int));
                break;

            case 's': {
                const char* s = va_arg(args, const char*);
                terminal_writestring(s ? s : "(null)");
                break;
            }

            case 'd': {
                int32_t val = va_arg(args, int32_t);
                if (val < 0) {
                    terminal_putchar('-');
                    /* Cast to uint32_t to handle INT32_MIN correctly */
                    print_uint((uint32_t)(-val), 10);
                } else {
                    print_uint((uint32_t)val, 10);
                }
                break;
            }

            case 'u':
                print_uint(va_arg(args, uint32_t), 10);
                break;

            case 'x':
                print_uint(va_arg(args, uint32_t), 16);
                break;

            case '%':
                terminal_putchar('%');
                break;

            default:
                /* Unknown specifier: print literally */
                terminal_putchar('%');
                terminal_putchar(fmt[i]);
                break;
        }
    }

    va_end(args);
}
