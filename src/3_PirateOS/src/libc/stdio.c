#include "libc/libs.h"
#include "arch/i386/io.h"

// VGA text mode exposes the visible 80x25 character grid at physical address 0xB8000.
// Writing a 16-bit entry here immediately updates what the user sees on screen.
static uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
static size_t terminal_row = 0;                          // Current row in the terminal
static size_t terminal_column = 0;                       // Current column in the terminal
static size_t terminal_view_top = 0;                     // First visible row in scrollback
static uint8_t terminal_color = 0x0F;                    // Terminal color
static int terminal_cursor_visible = 0;
// Keep a larger software buffer than the visible VGA window so the keyboard driver
// can scroll through older output without losing it immediately.
static uint16_t terminal_buffer[512][80];

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define TERMINAL_SCROLLBACK_LINES 512
#define VGA_CRTC_INDEX_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 0x3D5
#define VGA_CURSOR_START_REGISTER 0x0A
#define VGA_CURSOR_END_REGISTER 0x0B
#define VGA_CURSOR_LOCATION_HIGH_REGISTER 0x0E
#define VGA_CURSOR_LOCATION_LOW_REGISTER 0x0F

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8; // Combine character and color into a single 16-bit value
}

static void terminal_enable_cursor(void)
{
    uint8_t start;
    uint8_t end;

    // Program the VGA CRT controller to show the hardware text cursor with a
    // sensible scanline shape near the bottom of each character cell.
    outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_START_REGISTER);
    start = inb(VGA_CRTC_DATA_PORT);
    outb(VGA_CRTC_DATA_PORT, (start & 0xC0) | 13);

    outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_END_REGISTER);
    end = inb(VGA_CRTC_DATA_PORT);
    outb(VGA_CRTC_DATA_PORT, (end & 0xE0) | 15);

    terminal_cursor_visible = 1;
}

static void terminal_disable_cursor(void)
{
    // Bit 5 disables the VGA text cursor entirely.
    outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_START_REGISTER);
    outb(VGA_CRTC_DATA_PORT, 0x20);
    terminal_cursor_visible = 0;
}

static void terminal_set_cursor_position(size_t row, size_t column)
{
    uint16_t position = (uint16_t)(row * VGA_WIDTH + column);

    // The VGA hardware cursor is addressed as a linear cell index inside the
    // currently visible 80x25 text page.
    outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_LOCATION_LOW_REGISTER);
    outb(VGA_CRTC_DATA_PORT, (uint8_t)(position & 0xFF));
    outb(VGA_CRTC_INDEX_PORT, VGA_CURSOR_LOCATION_HIGH_REGISTER);
    outb(VGA_CRTC_DATA_PORT, (uint8_t)((position >> 8) & 0xFF));
}

static size_t terminal_max_view_top(void)
{
    if (terminal_row < VGA_HEIGHT) {
        return 0;
    }

    return terminal_row - (VGA_HEIGHT - 1);
}

static int terminal_view_is_at_bottom(void)
{
    return terminal_view_top == terminal_max_view_top();
}

static void terminal_clear_buffer_row(size_t row)
{
    size_t column;

    for (column = 0; column < VGA_WIDTH; column++) {
        terminal_buffer[row][column] = vga_entry(' ', terminal_color);
    }
}

static void terminal_render_view(void)
{
    size_t row;
    size_t column;
    int cursor_is_visible;
    size_t visible_cursor_row;

    // Copy the selected scrollback window from the software buffer into the real
    // VGA text buffer. This is the point where terminal state becomes visible.
    for (row = 0; row < VGA_HEIGHT; row++) {
        for (column = 0; column < VGA_WIDTH; column++) {
            VGA_MEMORY[row * VGA_WIDTH + column] =
                terminal_buffer[terminal_view_top + row][column];
        }
    }

    cursor_is_visible = terminal_row >= terminal_view_top &&
                        terminal_row < terminal_view_top + VGA_HEIGHT;

    if (!cursor_is_visible) {
        if (terminal_cursor_visible) {
            terminal_disable_cursor();
        }
        return;
    }

    if (!terminal_cursor_visible) {
        terminal_enable_cursor();
    }

    visible_cursor_row = terminal_row - terminal_view_top;
    terminal_set_cursor_position(visible_cursor_row, terminal_column);
}

static void terminal_shift_scrollback_up(void)
{
    size_t row;
    size_t column;

    // Once the software scrollback is full, discard the oldest line and move
    // everything else up so new output can continue at the bottom.
    for (row = 1; row < TERMINAL_SCROLLBACK_LINES; row++) {
        for (column = 0; column < VGA_WIDTH; column++) {
            terminal_buffer[row - 1][column] = terminal_buffer[row][column];
        }
    }

    terminal_clear_buffer_row(TERMINAL_SCROLLBACK_LINES - 1);

    if (terminal_view_top > 0) {
        terminal_view_top--;
    }
}

static void terminal_new_line(void)
{
    int follow_bottom = terminal_view_is_at_bottom();

    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= TERMINAL_SCROLLBACK_LINES) {
        terminal_shift_scrollback_up();
        terminal_row = TERMINAL_SCROLLBACK_LINES - 1;
    }

    if (follow_bottom) {
        terminal_view_top = terminal_max_view_top();
    }

    terminal_render_view();
}

void terminal_initialize(void)
{
    size_t row;

    // Reset both the visible terminal state and the off-screen scrollback store
    // before drawing the first frame into VGA memory.
    terminal_row = 0;
    terminal_column = 0;
    terminal_view_top = 0;
    terminal_color = 0x0F;
    terminal_cursor_visible = 0;

    for (row = 0; row < TERMINAL_SCROLLBACK_LINES; row++) {
        terminal_clear_buffer_row(row);
    }

    terminal_render_view();
}

void terminal_refresh(void)
{
    terminal_render_view();
}

void terminal_set_color(uint8_t color)
{
    terminal_color = color;
}

uint8_t terminal_get_color(void)
{
    return terminal_color;
}

void terminal_scroll_line_up(void)
{
    if (terminal_view_top > 0) {
        terminal_view_top--;
        terminal_render_view();
    }
}

void terminal_scroll_line_down(void)
{
    size_t max_view_top = terminal_max_view_top();

    if (terminal_view_top < max_view_top) {
        terminal_view_top++;
        terminal_render_view();
    }
}

void terminal_scroll_page_up(void)
{
    if (terminal_view_top > VGA_HEIGHT) {
        terminal_view_top -= VGA_HEIGHT;
    } else {
        terminal_view_top = 0;
    }

    terminal_render_view();
}

void terminal_scroll_page_down(void)
{
    size_t max_view_top = terminal_max_view_top();

    terminal_view_top += VGA_HEIGHT;
    if (terminal_view_top > max_view_top) {
        terminal_view_top = max_view_top;
    }

    terminal_render_view();
}

void terminal_scroll_to_top(void)
{
    terminal_view_top = 0;
    terminal_render_view();
}

void terminal_scroll_to_bottom(void)
{
    terminal_view_top = terminal_max_view_top();
    terminal_render_view();
}

int putchar(int ic) // Writes a character to the terminal
{
    unsigned char c = (unsigned char)ic;
    int follow_bottom = terminal_view_is_at_bottom();

    if (c == '\n')
    {
        terminal_new_line();
        return ic;
    }

    if (c == '\b')
    {
        // Backspace edits the software buffer first, then re-renders the active
        // view so the visible VGA page stays in sync.
        if (terminal_column > 0)
        {
            terminal_column--;
        }
        else if (terminal_row > 0)
        {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }

        terminal_buffer[terminal_row][terminal_column] = vga_entry(' ', terminal_color);
        if (follow_bottom) {
            terminal_view_top = terminal_max_view_top();
        }
        terminal_render_view();
        return ic;
    }

    terminal_buffer[terminal_row][terminal_column] = vga_entry(c, terminal_color);

    // We write into the software terminal buffer and only then refresh VGA memory.
    // That keeps scrolling/history behavior independent from the physical screen.
    terminal_column++;
    if (terminal_column == VGA_WIDTH)
    {
        terminal_new_line();
    } else {
        if (follow_bottom) {
            terminal_view_top = terminal_max_view_top();
        }
        terminal_render_view();
    }

    return ic;
}

static void print_number(int value)
{
    char buffer[16];
    int i = 0;

    if (value == 0)
    {
        putchar('0');
        return;
    }

    if (value < 0)
    {
        putchar('-');
        value = -value;
    }

    while (value > 0)
    {
        buffer[i++] = '0' + (value % 10); // Store the last digit as a character in the buffer
        value /= 10;
    }

    while (i > 0)
    {
        putchar(buffer[--i]);
    }
}

int printf(const char *format, ...) // A simple implementation of printf that supports %d, %s, and %c format specifiers
{
    va_list args;
    va_start(args, format);

    int written = 0;

    for (size_t i = 0; format[i] != '\0'; i++) // Iterate through each character in the format string
    {

        if (format[i] == '%') // Check for format specifier
        {
            i++;

            if (format[i] == 'd')
            {
                int value = va_arg(args, int);
                print_number(value);
            }
            else if (format[i] == 's')
            {
                char *str = va_arg(args, char *); // Get the string argument and print it character by character
                while (*str)
                {
                    putchar(*str++);
                }
            }
            else if (format[i] == 'c')
            {
                char c = (char)va_arg(args, int);
                putchar(c);
            }
        }
        else
        {
            putchar(format[i]);
        }

        written++;
    }

    va_end(args); // Clean up the variable argument list
    return written;
}
