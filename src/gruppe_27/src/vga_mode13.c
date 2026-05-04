#include "vga_mode13.h"
#include "common.h"

#define VGA_MISC_WRITE   0x3C2
#define VGA_SEQ_INDEX    0x3C4
#define VGA_SEQ_DATA     0x3C5
#define VGA_GC_INDEX     0x3CE
#define VGA_GC_DATA      0x3CF
#define VGA_AC_INDEX     0x3C0
#define VGA_AC_WRITE     0x3C0
#define VGA_CRTC_INDEX   0x3D4
#define VGA_CRTC_DATA    0x3D5
#define VGA_INSTAT_READ  0x3DA

/* ── Mode 13h ──────────────────────────────────────────────────────── */

static uint8_t vga_font_buffer[4096];
static const uint8_t mode13_misc = 0x63;
static const uint8_t mode13_seq[5] = {
    0x03, 0x01, 0x0F, 0x00, 0x0E
};
static const uint8_t mode13_crtc[25] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF
};
static const uint8_t mode13_gc[9] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF
};
static const uint8_t mode13_ac[21] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

/* ── Tekstmodus 3 (80x25) ─────────────────────────────────────────── */

static const uint8_t text3_misc = 0x67;
static const uint8_t text3_seq[5] = {
    0x03, 0x00, 0x03, 0x00, 0x02
};
static const uint8_t text3_crtc[25] = {
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF
};
static const uint8_t text3_gc[9] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF
};
static const uint8_t text3_ac[21] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

/* ── Felles skrivefunksjon ────────────────────────────────────────── */

static void write_vga_regs(uint8_t misc, const uint8_t *seq, const uint8_t *crtc, const uint8_t *gc, const uint8_t *ac) {
    __asm__("cli"); // Disable interrupts during VGA register programming
    outb(VGA_MISC_WRITE, misc);

    for (uint8_t i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA,  seq[i]);
    }

    /* Lås opp CRTC reg 0-7 */
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA,  inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA,  inb(VGA_CRTC_DATA) & ~0x80);

    for (uint8_t i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA,  crtc[i]);
    }

    for (uint8_t i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA,  gc[i]);
    }

    /* Les 0x3DA for å resette AC flip-flop */
    inb(VGA_INSTAT_READ);
    for (uint8_t i = 0; i < 21; i++) {
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, ac[i]);
    }
    outb(VGA_AC_INDEX, 0x20);
    __asm__("sti"); // Re-enable interrupts after programming
}

/* ── Public API ───────────────────────────────────────────────────── */

void vga_enter_mode13() {
    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, 0x02); // Read map select = 2
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, 0x00); // Disable odd/even
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, 0x00); // Map to 0xA0000

    // 2. Copy the font data out
    uint8_t *vga_mem = (uint8_t*)0xA0000;
    for (int i = 0; i < 4096; i++) {
        vga_font_buffer[i] = vga_mem[i];
    }

    // 3. Restore default GC settings so we don't break Mode 13h
    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, 0x10);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, 0x0E);
    write_vga_regs(mode13_misc, mode13_seq, mode13_crtc, mode13_gc, mode13_ac);
    
}

void vga_exit_mode13() {

    write_vga_regs(text3_misc, text3_seq, text3_crtc, text3_gc, text3_ac);
    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, 0x04); // Write to plane 2
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, 0x06); // Sequential access
    outb(VGA_GC_INDEX,  0x04); outb(VGA_GC_DATA, 0x02); // Read plane 2
    outb(VGA_GC_INDEX,  0x05); outb(VGA_GC_DATA, 0x00); // No odd/even
    outb(VGA_GC_INDEX,  0x06); outb(VGA_GC_DATA, 0x00); // Map to 0xA0000

    // 2. Push the saved font back in
    uint8_t *vga_mem = (uint8_t*)0xA0000;
    for (int i = 0; i < 4096; i++) {
        vga_mem[i] = vga_font_buffer[i];
    }

    // 3. Restore Sequencer/GC to "Text Mode" defaults
    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, 0x03);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, 0x02);
    outb(VGA_GC_INDEX,  0x04); outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX,  0x05); outb(VGA_GC_DATA, 0x10);
    outb(VGA_GC_INDEX,  0x06); outb(VGA_GC_DATA, 0x0E);

}

void vga_put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= VGA_MODE13_WIDTH)  return;
    if (y < 0 || y >= VGA_MODE13_HEIGHT) return;
    VGA_FRAMEBUFFER[y * VGA_MODE13_WIDTH + x] = color;
}

void vga_clear(uint8_t color) {
    uint8_t *fb = VGA_FRAMEBUFFER;
    for (int i = 0; i < VGA_MODE13_WIDTH * VGA_MODE13_HEIGHT; i++)
        fb[i] = color;
}
uint8_t vga_get_pixel(int x, int y) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) return 0;
    return VGA_FRAMEBUFFER[y * 320 + x];
}

// Bresenham's line algorithm
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        vga_put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}