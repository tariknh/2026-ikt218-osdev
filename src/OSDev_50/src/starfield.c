#include "starfield.h"
#include <libc/system.h>
#include <libc/stdint.h>
#include <keyboard.h>
#include <pit.h>
#include <memory.h>

#define VGA_W 80
#define VGA_H 25
#define VGA ((volatile uint16_t*)0xB8000)

#define N_STARS 140

typedef struct {
    int x, y;
    int speed;   // 1..3
} Star;

static uint32_t rng = 1;
static uint32_t rand_u32(void) {
    rng = rng * 1664525u + 1013904223u;
    return rng;
}

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static void vga_clear(uint8_t color) {
    for (int y = 0; y < VGA_H; y++) {
        for (int x = 0; x < VGA_W; x++) {
            VGA[y * VGA_W + x] = vga_entry(' ', color);
        }
    }
}

static void vga_put(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= VGA_W || y < 0 || y >= VGA_H) return;
    VGA[y * VGA_W + x] = vga_entry(c, color);
}

static void vga_text(int x, int y, const char* s, uint8_t color) {
    for (int i = 0; s[i] && (x + i) < VGA_W; i++) {
        vga_put(x + i, y, s[i], color);
    }
}

static void init_stars(Star* stars, int n) {
    for (int i = 0; i < n; i++) {
        stars[i].x = (int)(rand_u32() % VGA_W);
        stars[i].y = 1 + (int)(rand_u32() % (VGA_H - 1));
        stars[i].speed = 1 + (int)(rand_u32() % 3);
    }
}

static void clear_playfield(uint8_t color) {
    for (int y = 1; y < VGA_H; y++) {
        for (int x = 0; x < VGA_W; x++) {
            VGA[y * VGA_W + x] = vga_entry(' ', color);
        }
    }
}

static char hex_digit(uint8_t v) {
    v &= 0xF;
    return (v < 10) ? ('0' + v) : ('A' + (v - 10));
}

void starfield_run(void) {
    Star* stars = (Star*)malloc(sizeof(Star) * N_STARS);
    static Star fallback[N_STARS];
    if (!stars) stars = fallback;

    int delay_ms = 50;
    uint8_t color = 0x0F;

    vga_clear(color);
    vga_text(0, 0, "Starfield: Q quit | R reset | W faster | S slower", 0x0F);

    init_stars(stars, N_STARS);
    asm volatile ("sti"); 

    while (1) {
        uint8_t sc = keyboard_last_scancode();

        uint32_t n = keyboard_irq_count();

        vga_put(70, 0, hex_digit((n >> 12) & 0xF), 0x0F);
        vga_put(71, 0, hex_digit((n >>  8) & 0xF), 0x0F);
        vga_put(72, 0, hex_digit((n >>  4) & 0xF), 0x0F);
        vga_put(73, 0, hex_digit((n >>  0) & 0xF), 0x0F);

        if (sc) {
            vga_put(76, 0, '0', 0x0F);
            vga_put(77, 0, 'x', 0x0F);
            vga_put(78, 0, hex_digit(sc >> 4), 0x0F);
            vga_put(79, 0, hex_digit(sc),      0x0F);
        }
        if (sc == 0x10 || sc == 0x15) break;

        if (sc == 0x13 || sc == 0x2D) init_stars(stars, N_STARS);

        if (sc == 0x11 || sc == 0x1D) {
            if (delay_ms > 10) delay_ms -= 10;
        }

        if (sc == 0x1F || sc == 0x1B) {
            if (delay_ms < 200) delay_ms += 10;
        }

        clear_playfield(color);

        for (int i = 0; i < N_STARS; i++) {
            stars[i].y += stars[i].speed;
            if (stars[i].y >= VGA_H) {
                stars[i].y = 1;
                stars[i].x = (int)(rand_u32() % VGA_W);
                stars[i].speed = 1 + (int)(rand_u32() % 3);
            }
            vga_put(stars[i].x, stars[i].y, '.', 0x0F);
        }

        sleep_interrupt((uint32_t)delay_ms);
        asm volatile ("sti");
    }

    if (stars != fallback) free(stars);
    vga_clear(0x0F);
}