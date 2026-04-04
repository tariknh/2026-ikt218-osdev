#include "../../include/libc/stdio.h"  //
#include "../../include/libc/stdarg.h" //
#include "../../include/libc/stddef.h"
#include "../../include/libc/stdint.h"

#define VGA_WIDTH 80       // Width of screen, number of columns
#define VGA_HEIGHT 25      // Height of screen, number of rows
#define VGA_MEMORY 0xB8000 // Memory location of VGA memory buffer

static uint16_t *terminal_buffer =
    (uint16_t *)VGA_MEMORY;  // Terminal buffer, each index represents one
                             // character and one colour.
static int terminal_row = 0; // used to index the terminal buffer
static int terminal_col = 0; // used to index the terminal buffer
static uint8_t terminal_color = 0x07; // light grey on black.

// terminal_scroll moves every line up by one and cleans the last row.
static void terminal_scroll(uint8_t terminal_color) {
  for (int row = 1; row < VGA_HEIGHT; row++) {
    for (int col = 0; col < VGA_WIDTH; col++) {
      terminal_buffer[(row - 1) * VGA_WIDTH + col] =
          terminal_buffer[row * VGA_WIDTH + col];
    }
  }
  for (int col = 0; col < VGA_WIDTH; col++) {
    terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
        (terminal_color << 8) | ' ';
  }
  terminal_row = VGA_HEIGHT - 1;
}

// writes a single character to the VGA buffer.
// uses terminal_col and terminal_row for indexing and state.
void terminal_putchar(char c, uint8_t terminal_color) {
  if (c == '\n') {
    terminal_col = 0;
    terminal_row++;
  } else {
    int index = terminal_row * VGA_WIDTH + terminal_col;
    terminal_buffer[index] = (terminal_color << 8) | c;
    terminal_col++;

    if (terminal_col >= VGA_WIDTH) {
      terminal_col = 0;
      terminal_row++;
    }
  }

  if (terminal_row >= VGA_HEIGHT) {
    terminal_scroll(terminal_color);
  }
}

// Uses terminal_putchar to write strings.
static void terminal_write(const char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    terminal_putchar(str[i], terminal_color);
  }
}

// Helper function that formats integer values to text. and prints them to
// the screen
static void print_int(int value) {
  if (value < 0) {
    terminal_putchar('-', terminal_color);
    value = -value;
  }

  char buf[12];
  int i = 0;

  if (value == 0) {
    terminal_putchar('0', terminal_color);
    return;
  }

  while (value > 0) {
    // uses mod to get the last digit.
    // uses '0' to get starting index of numbers in ASCII which we use for our
    // lookup table
    buf[i++] = '0' + (value % 10);
    // divides by 10 so the number shift one place to the right.
    value /= 10;
  }

  while (i > 0) {
    terminal_putchar(buf[--i], terminal_color);
  }
}

// helper function for printing hex values
static void print_hex(uint32_t value) {
  char hex_chars[] = "0123456789ABCDEF";
  // always prints 0x
  terminal_write("0x");

  char buf[8];
  int i = 0;

  if (value == 0) {
    terminal_putchar('0', terminal_color);
    return;
  }

  while (value > 0) {
    // Gets the last 4 bits which is the same as one hex value.
    buf[i++] = hex_chars[value & 0xF];
    // shifts all bits down by four. discarding the four lower bits.
    // Preparing for next iteration of the while loop
    value >>= 4;
  }

  while (i > 0) {
    terminal_putchar(buf[--i], terminal_color);
  }
}

// Helper function for printf, so that we can have printf with and without
// default colour.
static int vprintf_color(const char *format, va_list args,
                         uint8_t terminal_color) {
  // loops through every character and selects the appropriate helper function
  // to use for printing.
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%' && format[i + 1] != '\0') {
      i++;
      switch (format[i]) {
      case 's':
        terminal_write(va_arg(args, const char *));
        break;
      case 'd':
        print_int(va_arg(args, int));
        break;
      case 'x':
        print_hex(va_arg(args, uint32_t));
        break;
      case 'c':
        terminal_putchar((char)va_arg(args, int), terminal_color);
        break;
      case '%':
        terminal_putchar('%', terminal_color);
        break;
      }
    } else {
      terminal_putchar(format[i], terminal_color);
    }
  }
}

// The main function this file exposes.
// Every other function is a helper function.
int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf_color(format, args, terminal_color);
  va_end(args);
  return 0;
}

int printf_color(const char *format, uint8_t terminal_color, ...) {
  // Uses va_list, va_start, va_end to have a undefined amount of arguments
  // given to the function.
  va_list args;
  va_start(args, terminal_color);
  vprintf_color(format, args, terminal_color);
  va_end(args);
  return 0;
}

// set specific index to some specific character and colour
void terminal_set_char(int x, int y, char c, uint8_t terminal_color) {
  if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT)
    return;
  terminal_buffer[y * VGA_WIDTH + x] = (terminal_color << 8) | c;
}

void clearTerminal() {
  for (int x = 0; x < VGA_WIDTH; x++) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
      terminal_set_char(x, y, ' ', terminal_color);
    }
  }
  terminal_row = 0;
  terminal_col = 0;
}

void terminal_write_color(const char *str, uint8_t terminal_color) {
  for (int i = 0; str[i] != '\0'; i++) {
    terminal_putchar(str[i], terminal_color);
  }
}
