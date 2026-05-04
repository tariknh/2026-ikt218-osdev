#include "terminal.h"
#include <libc/stdint.h>
#include <libc/stdarg.h>

// VGA text memory address
#define VGA_ADDRESS 0xB8000
// Screen width in text mode
#define VGA_COLS    80
// Screen height in text mode
#define VGA_ROWS    25
#define VGA_COLOR   0x0A  /* Light green on black */

// Screen buffer and cursor state
static volatile uint16_t *vga = (uint16_t *)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;
static int reserved_bottom_rows = 0;

// Count rows that can be used for normal output
static int terminal_printable_rows(void) {
    int rows = VGA_ROWS - reserved_bottom_rows;
    return (rows > 0) ? rows : 1;
}

/* Keep shell output visible by scrolling up when cursor passes last row. */
    // Do nothing if the cursor is still inside the screen
static void terminal_scroll_if_needed(void) {
    int rows = terminal_printable_rows();
    if (cursor_y < rows) {
        return;
    }

    /* Move rows 1..last_printable to 0..last_printable-1. */
    for (int y = 1; y < rows; y++) {
        for (int x = 0; x < VGA_COLS; x++) {
            vga[(y - 1) * VGA_COLS + x] = vga[y * VGA_COLS + x];
        }
    }

    /* Clear the last printable row. */
    for (int x = 0; x < VGA_COLS; x++) {
        vga[(rows - 1) * VGA_COLS + x] = (uint16_t)(VGA_COLOR << 8) | ' ';
    }

    cursor_y = rows - 1;
}

/* I/O helpers (replicated locally to avoid cross-file dependency) */
// Write one byte to a port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Read one byte from a port
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Load custom 8x16 glyph into VGA font memory (plane 2, char slot code). */
// Load one custom glyph into VGA font memory
static void vga_load_glyph(uint8_t code, const uint8_t glyph[16]) {
    /* Enter font-access mode */
    outb(0x3C4, 0x02); outb(0x3C5, 0x04); /* map mask: plane 2 */
    outb(0x3C4, 0x04); outb(0x3C5, 0x07); /* disable chain-4, odd/even */

    outb(0x3CE, 0x04); outb(0x3CF, 0x02); /* read plane 2 */
    outb(0x3CE, 0x05); outb(0x3CF, 0x00); /* write mode 0, disable odd/even */
    outb(0x3CE, 0x06); outb(0x3CF, 0x00); /* map starts at A000:0 */

    uint8_t *font = (uint8_t *)0xA0000;
    const uint32_t slot = (uint32_t)code * 32; /* 32 bytes per glyph slot */
    for (int i = 0; i < 16; i++) {
        font[slot + i] = glyph[i];
    }
    for (int i = 16; i < 32; i++) { /* zero padding */
        font[slot + i] = 0x00;
    }

    /* Restore default text mode mapping */
    outb(0x3C4, 0x02); outb(0x3C5, 0x03); /* map mask planes 0/1 */
    outb(0x3C4, 0x04); outb(0x3C5, 0x03); /* re-enable chain/odd-even */

    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x10);
    outb(0x3CE, 0x06); outb(0x3CF, 0x0E);
}

/* Minimal 8x16 bitmaps for ø/Ø placed in CP437 slots 0xF2/0xF3. 
   Feature request: Stein Erik Andersen */
// Lowercase oe glyph
static const uint8_t glyph_oe_lower[16] = {
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x3E, /* 00111110 */
    0x63, /* 01100011 */
    0x67, /* 01100111 */
    0x6B, /* 01101011 */
    0x73, /* 01110011 */
    0x3E, /* 00111110 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00  /* 00000000 */
};

// Uppercase oe glyph
static const uint8_t glyph_oe_upper[16] = {
    0x00, /* 00000000 */
    0x18, /* 00011000 */
    0x3C, /* 00111100 */
    0x66, /* 01100110 */
    0x6E, /* 01101110 */
    0x76, /* 01110110 */
    0x7E, /* 01111110 */
    0x6E, /* 01101110 */
    0x66, /* 01100110 */
    0x7E, /* 01111110 */
    0x3C, /* 00111100 */
    0x18, /* 00011000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00, /* 00000000 */
    0x00  /* 00000000 */
};

/* Clear the entire 80x25 text buffer and reset the cursor */
// Set up the terminal and install custom glyphs
void terminal_init(void) {
    /* Install custom glyphs for ø/Ø at CP437 slots 0xF2/0xF3. */
    vga_load_glyph(0xF2, glyph_oe_lower);
    vga_load_glyph(0xF3, glyph_oe_upper);

    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = (uint16_t)(VGA_COLOR << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

/* Clear without reloading glyphs */
// Clear the screen and keep the font loaded
void terminal_clear(void) {
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = (uint16_t)(VGA_COLOR << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

/* Move cursor to top-left without modifying contents */
// Put the cursor at the start of the screen
void terminal_home(void) {
    cursor_x = 0;
    cursor_y = 0;
}

// Draw one character at a fixed position
void terminal_put_at(int x, int y, char c) {
    if (x < 0 || x >= VGA_COLS || y < 0 || y >= VGA_ROWS) {
        return;
    }
    vga[y * VGA_COLS + x] = (uint16_t)(VGA_COLOR << 8) | (uint8_t)c;
}

// Draw one character with a custom color
void terminal_put_colored_at(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= VGA_COLS || y < 0 || y >= VGA_ROWS) {
        return;
    }
    vga[y * VGA_COLS + x] = (uint16_t)(color << 8) | (uint8_t)c;
}

// Return the screen width
int terminal_width(void) {
    return VGA_COLS;
}

// Return the screen height
int terminal_height(void) {
    return VGA_ROWS;
}

// Reserve rows at the bottom for other output
void terminal_reserve_bottom_rows(int rows) {
    if (rows < 0) rows = 0;
    if (rows >= VGA_ROWS) rows = VGA_ROWS - 1;
    reserved_bottom_rows = rows;
    if (cursor_y >= terminal_printable_rows()) {
        cursor_y = terminal_printable_rows() - 1;
        cursor_x = 0;
    }
}

/* Place a single character and advance the cursor (no scrolling yet) */
// Handle one output character
static void put_char(char c) {
    uint8_t ch = (uint8_t)c; /* avoid sign-extension for extended codes */
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        terminal_scroll_if_needed();
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_COLS - 1;
        } else {
            return;
        }
        int back_idx = cursor_y * VGA_COLS + cursor_x;
        vga[back_idx] = (uint16_t)(VGA_COLOR << 8) | ' ';
        return;
    }

    int index = cursor_y * VGA_COLS + cursor_x;
    vga[index] = (uint16_t)(VGA_COLOR << 8) | ch;

    if (++cursor_x >= VGA_COLS) {
        cursor_x = 0;
        cursor_y++;
        terminal_scroll_if_needed();
    }
}

/* Write a null-terminated string to the VGA text buffer */
// Print a string with the cursor logic
void terminal_write(const char *str) {
    while (*str) {
        put_char(*str++);
    }
}

/* Convert unsigned integer to string in a given base (2..16) */
// Convert one number to text
static void utoa(unsigned int value, unsigned int base, char *buf) {
    static const char digits[] = "0123456789abcdef";
    char tmp[32];
    int i = 0;
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (value && i < (int)sizeof(tmp)) {
        tmp[i++] = digits[value % base];
        value /= base;
    }
    for (int j = 0; j < i; j++) {
        buf[j] = tmp[i - j - 1];
    }
    buf[i] = '\0';
}

/* Minimal printf: supports %s, %c, %d, %u, %x, and %% */
// Print formatted text on the terminal
void terminal_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char numbuf[32];

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            put_char(*fmt);
            continue;
        }
        fmt++;  /* skip '%' */
        switch (*fmt) {
        case '%':
            put_char('%');
            break;
        case 'c':
            put_char((char)va_arg(ap, int));
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (s) terminal_write(s);
            else terminal_write("(null)");
            break;
        }
        case 'd': {
            int v = va_arg(ap, int);
            unsigned int uv = (v < 0) ? (unsigned int)(-v) : (unsigned int)v;
            utoa(uv, 10, numbuf);
            if (v < 0) put_char('-');
            terminal_write(numbuf);
            break;
        }
        case 'u': {
            unsigned int v = va_arg(ap, unsigned int);
            utoa(v, 10, numbuf);
            terminal_write(numbuf);
            break;
        }
        case 'x': {
            unsigned int v = va_arg(ap, unsigned int);
            utoa(v, 16, numbuf);
            terminal_write(numbuf);
            break;
        }
        default:
            /* Unknown specifier: print it literally */
            put_char('%');
            put_char(*fmt);
            break;
        }
    }

    va_end(ap);
}