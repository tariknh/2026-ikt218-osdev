#include "apps/raycaster/raycaster_internal.h"
#include "arch/i386/io.h"

#define VGA13_WIDTH 320
#define VGA13_HEIGHT 200

static uint8_t *const VGA13_MEMORY = (uint8_t *)0xA0000;
static uint16_t *const VGA_TEXT_MEMORY = (uint16_t *)0xB8000;
static uint8_t vga13_backbuffer[VGA13_WIDTH * VGA13_HEIGHT];
static uint8_t vga_text_font_backup[256 * 32];
static uint8_t vga_text_font_saved = 0;

#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX  0x3C4
#define VGA_SEQ_DATA   0x3C5
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_GC_INDEX   0x3CE
#define VGA_GC_DATA    0x3CF
#define VGA_AC_INDEX   0x3C0
#define VGA_AC_READ    0x3C1
#define VGA_AC_WRITE   0x3C0
#define VGA_INSTAT_READ 0x3DA
#define VGA_MISC_READ  0x3CC
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9

static const uint8_t vga_mode_13h_regs[] = {
    0x63,
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

static const uint8_t vga_mode_03h_regs[] = {
    0x67,
    0x03, 0x00, 0x03, 0x00, 0x02,
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x00, 0xFF,
    0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

static uint8_t vga_saved_text_regs[1 + 5 + 25 + 9 + 21];
static uint8_t vga_saved_text_regs_valid = 0;
static uint8_t vga_saved_dac_palette[256 * 3];
static uint8_t vga_saved_dac_palette_valid = 0;

// Read one value from VGA sequencer register
static uint8_t vga_read_seq(uint8_t index)
{
    outb(VGA_SEQ_INDEX, index);
    return inb(VGA_SEQ_DATA);
}

// Read one value from VGA graphics controller register
static uint8_t vga_read_gc(uint8_t index)
{
    outb(VGA_GC_INDEX, index);
    return inb(VGA_GC_DATA);
}

// Restore a standard text-mode 16-color palette
static void vga_restore_text_palette(void)
{
    static const uint8_t base_palette[16][3] = {
        {0, 0, 0}, {0, 0, 42}, {0, 42, 0}, {0, 42, 42},
        {42, 0, 0}, {42, 0, 42}, {42, 21, 0}, {42, 42, 42},
        {21, 21, 21}, {21, 21, 63}, {21, 63, 21}, {21, 63, 63},
        {63, 21, 21}, {63, 21, 63}, {63, 63, 21}, {63, 63, 63}
    };

    // Text mode attribute controller maps colors 0-7 to DAC 0-7 and
    // bright colors 8-15 to DAC 56-63 (0x38-0x3F).
    outb(VGA_DAC_WRITE_INDEX, 0);
    for (int i = 0; i < 16; i++) {
        outb(VGA_DAC_DATA, base_palette[i][0]);
        outb(VGA_DAC_DATA, base_palette[i][1]);
        outb(VGA_DAC_DATA, base_palette[i][2]);
    }

    outb(VGA_DAC_WRITE_INDEX, 56);
    for (int i = 8; i < 16; i++) {
        outb(VGA_DAC_DATA, base_palette[i][0]);
        outb(VGA_DAC_DATA, base_palette[i][1]);
        outb(VGA_DAC_DATA, base_palette[i][2]);
    }
}

// Write a full VGA register table for one video mode
static void vga_write_registers(const uint8_t *regs)
{
    outb(VGA_MISC_WRITE, *regs++);

    for (uint8_t i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs++);
    }

    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & (uint8_t)~0x80);

    for (uint8_t i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs++);
    }

    for (uint8_t i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs++);
    }

    for (uint8_t i = 0; i < 21; i++) {
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs++);
    }

    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

// Read current VGA register state into a buffer
static void vga_read_registers(uint8_t *regs)
{
    *regs++ = inb(VGA_MISC_READ);

    for (uint8_t i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        *regs++ = inb(VGA_SEQ_DATA);
    }

    for (uint8_t i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        *regs++ = inb(VGA_CRTC_DATA);
    }

    for (uint8_t i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        *regs++ = inb(VGA_GC_DATA);
    }

    for (uint8_t i = 0; i < 21; i++) {
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        *regs++ = inb(VGA_AC_READ);
    }

    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

// Save the full DAC palette so it can be restored later
static void vga_save_dac_palette(void)
{
    outb(VGA_DAC_READ_INDEX, 0);
    for (int i = 0; i < 256 * 3; i++) {
        vga_saved_dac_palette[i] = inb(VGA_DAC_DATA);
    }
    vga_saved_dac_palette_valid = 1;
}

// Restore the previously saved DAC palette
static void vga_restore_saved_dac_palette(void)
{
    if (!vga_saved_dac_palette_valid) {
        return;
    }

    outb(VGA_DAC_WRITE_INDEX, 0);
    for (int i = 0; i < 256 * 3; i++) {
        outb(VGA_DAC_DATA, vga_saved_dac_palette[i]);
    }
}

// Re-enable normal text VRAM mapping at B8000
static void vga_restore_text_memory_mapping(void)
{
    outb(VGA_SEQ_INDEX, 0x02);
    outb(VGA_SEQ_DATA, 0x03);
    outb(VGA_SEQ_INDEX, 0x04);
    outb(VGA_SEQ_DATA, 0x03);

    outb(VGA_GC_INDEX, 0x04);
    outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX, 0x05);
    outb(VGA_GC_DATA, 0x10);
    outb(VGA_GC_INDEX, 0x06);
    outb(VGA_GC_DATA, 0x0E);
}

// Fill the entire text buffer with blank characters
static void vga_clear_text_vram(void)
{
    for (int i = 0; i < 80 * 25; i++) {
        VGA_TEXT_MEMORY[i] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
}

// Force VGA settings so text output is visible
static void vga_force_text_output_visible(void)
{
    // Ensure display is not blanked by sequencer clocking mode.
    outb(VGA_SEQ_INDEX, 0x01);
    outb(VGA_SEQ_DATA, 0x00);

    // Ensure attribute controller is in text-compatible mode and enabled.
    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x10);
    outb(VGA_AC_WRITE, 0x0C);

    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x12);
    outb(VGA_AC_WRITE, 0x0F);

    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

// Reset text viewport and cursor registers to safe defaults
static void vga_fix_text_crtc_viewport(void)
{
    // Ensure text display starts at top-left of B800:0000.
    outb(VGA_CRTC_INDEX, 0x0C);
    outb(VGA_CRTC_DATA, 0x00);
    outb(VGA_CRTC_INDEX, 0x0D);
    outb(VGA_CRTC_DATA, 0x00);

    // Keep a sane cursor shape and origin position.
    outb(VGA_CRTC_INDEX, 0x0A);
    outb(VGA_CRTC_DATA, 0x0D);
    outb(VGA_CRTC_INDEX, 0x0B);
    outb(VGA_CRTC_DATA, 0x0F);
    outb(VGA_CRTC_INDEX, 0x0E);
    outb(VGA_CRTC_DATA, 0x00);
    outb(VGA_CRTC_INDEX, 0x0F);
    outb(VGA_CRTC_DATA, 0x00);
}

// Save text-mode font glyphs from VGA font plane
static void vga_save_text_font(void)
{
    volatile uint8_t *font_mem = (volatile uint8_t *)0xA0000;
    uint8_t old_seq2 = vga_read_seq(0x02);
    uint8_t old_seq4 = vga_read_seq(0x04);
    uint8_t old_gc4 = vga_read_gc(0x04);
    uint8_t old_gc5 = vga_read_gc(0x05);
    uint8_t old_gc6 = vga_read_gc(0x06);
    uint32_t nonzero_count = 0;

    // OSDev-tested font plane access sequence (plane 2 at A0000).
    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, 0x02);
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, 0x04);
    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, 0x04);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, 0x07);

    for (int i = 0; i < 256 * 32; i++) {
        uint8_t v = font_mem[i];
        vga_text_font_backup[i] = v;
        if (v != 0) {
            nonzero_count++;
        }
    }

    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, old_seq2);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, old_seq4);
    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, old_gc4);
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, old_gc5);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, old_gc6);

    vga_text_font_saved = (nonzero_count > 512) ? 1 : 0;
}

// Restore previously saved text-mode font glyphs
static void vga_restore_text_font(void)
{
    volatile uint8_t *font_mem = (volatile uint8_t *)0xA0000;
    uint8_t old_seq2;
    uint8_t old_seq4;
    uint8_t old_gc4;
    uint8_t old_gc5;
    uint8_t old_gc6;

    if (!vga_text_font_saved) {
        return;
    }

    old_seq2 = vga_read_seq(0x02);
    old_seq4 = vga_read_seq(0x04);
    old_gc4 = vga_read_gc(0x04);
    old_gc5 = vga_read_gc(0x05);
    old_gc6 = vga_read_gc(0x06);

    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, 0x02);
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, 0x00);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, 0x04);
    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, 0x04);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, 0x07);

    for (int i = 0; i < 256 * 32; i++) {
        font_mem[i] = vga_text_font_backup[i];
    }

    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, old_seq2);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, old_seq4);
    outb(VGA_GC_INDEX, 0x04); outb(VGA_GC_DATA, old_gc4);
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, old_gc5);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, old_gc6);
}

// Draw one pixel in the mode 13h backbuffer
static inline void vga13_put_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= VGA13_WIDTH || y < 0 || y >= VGA13_HEIGHT) {
        return;
    }
    vga13_backbuffer[y * VGA13_WIDTH + x] = color;
}

// Fill a rectangle in the mode 13h backbuffer
static void vga13_fill_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            vga13_put_pixel(xx, yy, color);
        }
    }
}

// Draw the top-right minimap and player marker
void raycaster_draw_minimap_mode13_internal(Raycaster *rc)
{
    const int map_px = 4;
    const int x0 = VGA13_WIDTH - (MAP_WIDTH * map_px) - 6;
    const int y0 = 6;

    vga13_fill_rect(x0 - 2, y0 - 2, MAP_WIDTH * map_px + 4, MAP_HEIGHT * map_px + 4, 0);

    for (int my = 0; my < MAP_HEIGHT; my++) {
        for (int mx = 0; mx < MAP_WIDTH; mx++) {
            uint8_t c = g_raycaster_map_data[my][mx] ? 13 : 2;
            vga13_fill_rect(x0 + mx * map_px, y0 + my * map_px, map_px - 1, map_px - 1, c);
        }
    }

    int px = x0 + (int)(rc->player.x * map_px);
    int py = y0 + (int)(rc->player.y * map_px);
    vga13_fill_rect(px - 1, py - 1, 3, 3, 15);
}

// Wait for vertical blank to reduce visible tearing
static void vga13_wait_vblank(void)
{
    while (inb(VGA_INSTAT_READ) & 0x08) {
    }
    while ((inb(VGA_INSTAT_READ) & 0x08) == 0) {
    }
}

// Cast one ray against the map grid and return hit distance
static double raycaster_cast_ray_fast(double from_x, double from_y, double angle)
{
    double ray_dir_x = raycaster_math_cos(angle);
    double ray_dir_y = raycaster_math_sin(angle);

    int map_x = (int)from_x;
    int map_y = (int)from_y;

    int step_x;
    int step_y;
    int side = 0;

    double side_dist_x;
    double side_dist_y;
    double abs_dir_x = (ray_dir_x < 0.0) ? -ray_dir_x : ray_dir_x;
    double abs_dir_y = (ray_dir_y < 0.0) ? -ray_dir_y : ray_dir_y;
    double delta_dist_x = (abs_dir_x < 0.000001) ? 1e30 : (1.0 / abs_dir_x);
    double delta_dist_y = (abs_dir_y < 0.000001) ? 1e30 : (1.0 / abs_dir_y);

    if (ray_dir_x < 0) {
        step_x = -1;
        side_dist_x = (from_x - (double)map_x) * delta_dist_x;
    } else {
        step_x = 1;
        side_dist_x = ((double)(map_x + 1) - from_x) * delta_dist_x;
    }

    if (ray_dir_y < 0) {
        step_y = -1;
        side_dist_y = (from_y - (double)map_y) * delta_dist_y;
    } else {
        step_y = 1;
        side_dist_y = ((double)(map_y + 1) - from_y) * delta_dist_y;
    }

    while (1) {
        if (side_dist_x < side_dist_y) {
            side_dist_x += delta_dist_x;
            map_x += step_x;
            side = 0;
        } else {
            side_dist_y += delta_dist_y;
            map_y += step_y;
            side = 1;
        }

        if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
            return MAX_DEPTH;
        }

        if (g_raycaster_map_data[map_y][map_x]) {
            double perp_dist;

            if (side == 0) {
                perp_dist = (((double)map_x - from_x) + (1 - step_x) * 0.5) / ray_dir_x;
            } else {
                perp_dist = (((double)map_y - from_y) + (1 - step_y) * 0.5) / ray_dir_y;
            }

            if (perp_dist < 0.0) {
                perp_dist = -perp_dist;
            }

            if (perp_dist > MAX_DEPTH) {
                return MAX_DEPTH;
            }

            return perp_dist;
        }
    }
}

// Switch VGA hardware to 320x200 256-color mode
void raycaster_vga13_set_mode_internal(void)
{
    asm volatile("cli");

    if (!vga_saved_text_regs_valid) {
        vga_read_registers(vga_saved_text_regs);
        vga_saved_text_regs_valid = 1;
    }

    if (!vga_saved_dac_palette_valid) {
        vga_save_dac_palette();
    }

    if (!vga_text_font_saved) {
        vga_save_text_font();
    }

    vga_write_registers(vga_mode_13h_regs);

    asm volatile("sti");
}

// Switch VGA hardware back to text mode and restore saved state
void raycaster_vga_text_set_mode_internal(void)
{
    asm volatile("cli");

    // Restore exact pre-game text-mode registers when available.
    if (vga_saved_text_regs_valid) {
        vga_write_registers(vga_saved_text_regs);
    } else {
        vga_write_registers(vga_mode_03h_regs);
        vga_write_registers(vga_mode_03h_regs);
    }

    // Always restore canonical text memory mapping, palette, and font.
    vga_restore_text_memory_mapping();
    vga_force_text_output_visible();
    vga_fix_text_crtc_viewport();
    if (vga_saved_dac_palette_valid) {
        vga_restore_saved_dac_palette();
    } else {
        vga_restore_text_palette();
    }
    vga_restore_text_font();
    vga_clear_text_vram();

    asm volatile("sti");
}

// Clear backbuffer with one color
void raycaster_vga13_clear_internal(uint8_t color)
{
    for (int i = 0; i < VGA13_WIDTH * VGA13_HEIGHT; i++) {
        vga13_backbuffer[i] = color;
    }
}

// Render one 3D frame into the mode 13h backbuffer
void raycaster_render_mode13_internal(Raycaster *rc)
{
    for (int x = 0; x < VGA13_WIDTH; x++) {
        double offset = (x - (VGA13_WIDTH / 2.0)) * (FOV / (double)VGA13_WIDTH);
        double ray_angle = rc->player.angle + offset;

        while (ray_angle < 0) ray_angle += 360;
        while (ray_angle >= 360) ray_angle -= 360;

        double raw_distance = raycaster_cast_ray_fast(rc->player.x, rc->player.y, ray_angle);
        double distance = raw_distance * raycaster_math_cos(offset);
        if (distance < 0.05) distance = 0.05;

        int wall_h = (int)(VGA13_HEIGHT / (distance + 0.5));
        if (wall_h > VGA13_HEIGHT) wall_h = VGA13_HEIGHT;

        int y0 = (VGA13_HEIGHT - wall_h) / 2;
        int y1 = y0 + wall_h;
        uint8_t wall_color = 12;

        if (distance > 5.0) wall_color = 14;
        if (distance > 9.0) wall_color = 7;

        if (y0 < 0) y0 = 0;
        if (y1 > VGA13_HEIGHT) y1 = VGA13_HEIGHT;

        for (int y = 0; y < y0; y++) {
            int p = y * VGA13_WIDTH + x;
            vga13_backbuffer[p] = 1;
        }

        for (int y = y0; y < y1; y++) {
            int p = y * VGA13_WIDTH + x;
            vga13_backbuffer[p] = wall_color;
        }

        for (int y = y1; y < VGA13_HEIGHT; y++) {
            int p = y * VGA13_WIDTH + x;
            vga13_backbuffer[p] = 8;
        }
    }
}

// Copy backbuffer to VGA memory after vblank
void raycaster_vga13_present_internal(void)
{
    vga13_wait_vblank();
    for (int i = 0; i < VGA13_WIDTH * VGA13_HEIGHT; i++) {
        VGA13_MEMORY[i] = vga13_backbuffer[i];
    }
}
