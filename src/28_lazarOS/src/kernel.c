#include <gdt.h>
#include <idt.h>


/* ───────────────────────────────────────────────────────────────
 * VGA Text-Mode Terminal
 *
 * The VGA text buffer is mapped at physical address 0xB8000.
 * Each cell is 2 bytes:
 *   byte 0 – ASCII character
 *   byte 1 – colour attribute (foreground | background << 4)
 * The standard text screen is 80 columns × 25 rows.
 * ─────────────────────────────────────────────────────────────── */

#define VGA_ADDRESS  0xB8000
#define VGA_COLS     80
#define VGA_ROWS     25

/* Colour byte: white (0xF) text on black (0x0) background */
#define VGA_COLOR    0x0F

static volatile unsigned short *vga = (volatile unsigned short *)VGA_ADDRESS;

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void serial_init(void)
{
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static void serial_write_char(char c)
{
    outb(0x3F8, (unsigned char)c);
}

static void serial_write(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n')
            serial_write_char('\r');
        serial_write_char(str[i]);
    }
}

/* Current cursor position */
static int terminal_col = 0;
static int terminal_row = 0;

/* terminal_clear – fill every cell with a blank space */
static void terminal_clear(void)
{
    int row;
    int col;

    for (row = 0; row < VGA_ROWS; row++)
        for (col = 0; col < VGA_COLS; col++)
            vga[row * VGA_COLS + col] = (unsigned short)(' ' | (VGA_COLOR << 8));

    terminal_col = 0;
    terminal_row = 0;
}

/* terminal_scroll – shift every row up by one, blank the last row */
static void terminal_scroll(void)
{
    int row;
    int col;

    for (row = 1; row < VGA_ROWS; row++)
        for (col = 0; col < VGA_COLS; col++)
            vga[(row - 1) * VGA_COLS + col] = vga[row * VGA_COLS + col];

    /* Blank the bottom row */
    for (col = 0; col < VGA_COLS; col++)
        vga[(VGA_ROWS - 1) * VGA_COLS + col] =
            (unsigned short)(' ' | (VGA_COLOR << 8));

    terminal_row = VGA_ROWS - 1;
}

/* terminal_putchar – write a single character to the screen */
void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_col = 0;
    } else {
        vga[terminal_row * VGA_COLS + terminal_col] =
            (unsigned short)((unsigned char)c | (VGA_COLOR << 8));
        terminal_col++;

        /* Wrap at right edge */
        if (terminal_col >= VGA_COLS) {
            terminal_col = 0;
            terminal_row++;
        }
    }

    /* Scroll if we have gone past the last row */
    if (terminal_row >= VGA_ROWS)
        terminal_scroll();
}

/* terminal_write – write a null-terminated string to the screen */
void terminal_write(const char *str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}

/* ───────────────────────────────────────────────────────────────
 * Kernel Entry Point
 * ─────────────────────────────────────────────────────────────── */
void main(void)
{
    serial_init();
    serial_write("kernel: entered main\n");

    /* 1. Set up the Global Descriptor Table */
    gdt_init();
    serial_write("kernel: gdt loaded\n");

    //setup IDT
    idt_init();
    serial_write("kernel: idt loaded\n");

    /* 2. Initialise the text-mode terminal */
    terminal_clear();

    /* 3. Print the required greeting */
    terminal_write("Hello World\n");
    serial_write("Hello World\n");

    /* Halt – the kernel has nothing more to do */
    while (1) {
    }
}