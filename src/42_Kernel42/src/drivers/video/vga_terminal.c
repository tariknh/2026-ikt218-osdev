#include <stdbool.h>

#include "arch/i386/cpu/ports.h"
#include "drivers/video/vga_text.h"
#include "libc/stdint.h"
#include "libc/string.h"

static size_t text_row;
static size_t text_column;
static uint8_t text_color;
static uint16_t *text_buffer = (uint16_t *) VGA_TEXT_MEMORY;
static bool vga_debug_serial = false;

void vga_text_enable_debug_serial(bool enable) {
  vga_debug_serial = enable;
}

static void serial_putchar(char c) {
  port_byte_out(0xE9, (uint8_t) c);
}

static uint16_t vga_entry(char uc, uint8_t color) {
  return (uint16_t) uc | (uint16_t) color << 8;
}

void vga_text_initialise(void) {
  vga_text_clear_screen();
}

void vga_text_clear_screen(void) {
  text_row = 0;
  text_column = 0;
  text_color = vga_text_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  for (size_t y = 0; y < VGA_TEXT_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
      const size_t index = y * VGA_TEXT_WIDTH + x;
      text_buffer[index] = vga_entry(' ', text_color);
    }
  }
}

void vga_text_setcolor(uint8_t color) {
  text_color = color;
}

void vga_text_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_TEXT_WIDTH + x;
  text_buffer[index] = vga_entry(c, color);
}

#define HTAB_WIDTH 4
#define VTAB_WIDTH 4

void vga_text_putchar(char c) {
  if (vga_debug_serial) {
    serial_putchar(c);
  }
  // handle ANSI control characters and cursor
  switch (c) {
    case '\n': {
      const size_t index = text_row * VGA_TEXT_WIDTH + text_column;
      text_buffer[index] = vga_entry(' ', text_color);

      text_column = 0;
      if (text_row < VGA_TEXT_HEIGHT - 1) {
        text_row++;
      } else {
        for (size_t y = 0; y < VGA_TEXT_HEIGHT - 1; y++) {
          for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
            text_buffer[y * VGA_TEXT_WIDTH + x] = text_buffer[(y + 1) * VGA_TEXT_WIDTH + x];
          }
        }
        size_t last_row = (VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH;
        for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
          text_buffer[last_row + x] = vga_entry(' ', text_color);
        }
      }
      return;
    }
    case '\t': {
      size_t offset = HTAB_WIDTH - (text_column % HTAB_WIDTH);
      text_column += offset ? offset : HTAB_WIDTH;
      return;
    }
    case '\r': {
      text_column = 0;
      return;
    }
    case '\f': {
      vga_text_clear_screen();
      return;
    }
    case '\b': {
      if (text_column != 0)
        text_column--;
      return;
    }
    case '\v': {
      size_t offset = VTAB_WIDTH - (text_row % VTAB_WIDTH);
      text_row += offset ? offset : VTAB_WIDTH;
      return;
    }
    case '\a': {
      return;
    }
    default:
      break;
  }

  const size_t index = text_row * VGA_TEXT_WIDTH + text_column;
  text_buffer[index] = vga_entry(c, text_color);

  text_column++;
  if (text_column >= VGA_TEXT_WIDTH) {
    text_column = 0;
    if (text_row < VGA_TEXT_HEIGHT - 1) {
      text_row++;
    } else {
      for (size_t y = 0; y < VGA_TEXT_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
          text_buffer[y * VGA_TEXT_WIDTH + x] = text_buffer[(y + 1) * VGA_TEXT_WIDTH + x];
        }
      }
      size_t last_row = (VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH;
      for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
        text_buffer[last_row + x] = vga_entry(' ', text_color);
      }
    }
  }
}

void vga_text_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    vga_text_putchar(data[i]);
}

void vga_text_writestring(const char *data) {
  vga_text_write(data, strlen(data));
}

void vga_text_scroll(void) {
  for (size_t y = 0; y < VGA_TEXT_HEIGHT - 1; y++) {
    for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
      text_buffer[y * VGA_TEXT_WIDTH + x] = text_buffer[(y + 1) * VGA_TEXT_WIDTH + x];
    }
  }

  size_t last_row = (VGA_TEXT_HEIGHT - 1) * VGA_TEXT_WIDTH;
  for (size_t x = 0; x < VGA_TEXT_WIDTH; x++) {
    text_buffer[last_row + x] = vga_entry(' ', text_color);
  }
}

uint16_t vga_text_get_entry_at(size_t x, size_t y) {
  return text_buffer[y * VGA_TEXT_WIDTH + x];
}

uint8_t vga_text_get_color(void) {
  return text_color;
}

void vga_text_get_cursor_position(size_t *x, size_t *y) {
  *x = text_column;
  *y = text_row;
}

#define VGA_CRTC_ADDRESS_PORT_COLOR 0x3D4
#define VGA_CRTC_DATA_PORT_COLOR 0x3D5
#define VGA_CRTC_CURSOR_START_INDEX 0x0A
#define VGA_CRTC_CURSOR_DISABLE_MASK 0x20

void vga_text_disable_cursor() {
  port_byte_out(VGA_CRTC_ADDRESS_PORT_COLOR, VGA_CRTC_CURSOR_START_INDEX);
  port_byte_out(VGA_CRTC_DATA_PORT_COLOR, VGA_CRTC_CURSOR_DISABLE_MASK);
}
