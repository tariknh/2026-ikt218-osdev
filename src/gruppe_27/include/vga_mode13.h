#ifndef VGA_MODE13_H
#define VGA_MODE13_H

#include <stdint.h>

#define VGA_MODE13_WIDTH   320
#define VGA_MODE13_HEIGHT  200
#define VGA_FRAMEBUFFER    ((uint8_t*)0xA0000)

void vga_enter_mode13();
void vga_exit_mode13();   // returns to text mode 3 (80x25)
void vga_put_pixel(int x, int y, uint8_t color);
void vga_clear(uint8_t color);
uint8_t vga_get_pixel(int x, int y);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);

#endif