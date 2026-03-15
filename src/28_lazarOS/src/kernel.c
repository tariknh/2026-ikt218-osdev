#include <gdt.h>

// TODO: Define VGA address (0xB8000), cols (80), rows (25), color (0x0F)
// TODO: Implement terminal_clear()
// TODO: Implement terminal_putchar() with newline + scroll support
// TODO: Implement terminal_write()

void main(void)
{
    // TODO: Call gdt_init()
    // TODO: Call terminal_clear()
    // TODO: Call terminal_write("Hello World\n")

    while (1)
        asm volatile("hlt");
}

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

/* Current cursor position */
static int terminal_col = 0;
static int terminal_row = 0;

/* terminal_clear – fill every cell with a blank space */
static void terminal_clear(void)
{
    for (int row = 0; row < VGA_ROWS; row++)
        for (int col = 0; col < VGA_COLS; col++)
            vga[row * VGA_COLS + col] = (unsigned short)(' ' | (VGA_COLOR << 8));

    terminal_col = 0;
    terminal_row = 0;
}

/* terminal_scroll – shift every row up by one, blank the last row */
static void terminal_scroll(void)
{
    for (int row = 1; row < VGA_ROWS; row++)
        for (int col = 0; col < VGA_COLS; col++)
            vga[(row - 1) * VGA_COLS + col] = vga[row * VGA_COLS + col];

    /* Blank the bottom row */
    for (int col = 0; col < VGA_COLS; col++)
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
    for (int i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}

/* ───────────────────────────────────────────────────────────────
 * Kernel Entry Point
 * ─────────────────────────────────────────────────────────────── */
void main(void)
{
    /* 1. Set up the Global Descriptor Table */
    gdt_init();

    /* 2. Initialise the text-mode terminal */
    terminal_clear();

    /* 3. Print the required greeting */
    terminal_write("Hello World\n");

    /* Halt – the kernel has nothing more to do */
    while (1)
        asm volatile("hlt");
}