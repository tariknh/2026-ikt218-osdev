#include "mouse.h"
#include "idt.h"
#include "common.h"
#include "vga_mode13.h"

#define MOUSE_STATUS_PORT  0x64
#define MOUSE_DATA_PORT    0x60
#define CURSOR_COLOR       15 
#define DRAW_COLOR         1  

static int mouse_x     = 160;
static int mouse_y     = 100;
static int left_button  = 0;
static int right_button = 0;
static int prev_x = 160;
static int prev_y = 100;
static uint8_t saved_pixel_color = 0;

volatile int drawing_mode_active = 0;
static volatile int right_clicked = 0;  

static void mouse_wait_write() {
    int t = 100000;
    while (t-- && (inb(MOUSE_STATUS_PORT) & 0x02));
}

static void mouse_wait_read() {
    int t = 100000;
    while (t-- && !(inb(MOUSE_STATUS_PORT) & 0x01));
}

static void mouse_write(uint8_t val) {
    mouse_wait_write();
    outb(MOUSE_STATUS_PORT, 0xD4);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, val);
}

static uint8_t mouse_read() {
    mouse_wait_read();
    return inb(MOUSE_DATA_PORT);
}

static uint8_t packet[3];
static int     cycle = 0;

static void mouse_handler(struct registers *r) {
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if (!(status & 0x20)) return; 

    packet[cycle] = inb(MOUSE_DATA_PORT);
    cycle++;

    if (cycle < 3) return; // Wait for full 3-byte packet
    cycle = 0;

    if (right_button = (packet[0] & 0x02)) {
        right_clicked = 1;
        return;
    }

    if (!drawing_mode_active) return;

    // --- STEP 1: RESTORE ---
    // Put back exactly what was there before the cursor showed up
    vga_put_pixel(prev_x, prev_y, saved_pixel_color);

    // --- STEP 2: CALCULATE ---
    int dx = packet[1];
    int dy = packet[2];
    if (packet[0] & 0x10) dx |= 0xFFFFFF00;
    if (packet[0] & 0x20) dy |= 0xFFFFFF00;

    mouse_x += dx;
    mouse_y -= dy;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= 319) mouse_x = 319; 
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y >= 199) mouse_y = 199;

    // --- STEP 3: PAINT ---
    left_button = packet[0] & 0x01;
    if (left_button) {
        vga_draw_line(prev_x, prev_y, mouse_x, mouse_y, DRAW_COLOR);
    }

    // --- STEP 4: SAVE & DRAW CURSOR ---
    // Remember what is at the NEW position (even if we just painted it)
    saved_pixel_color = vga_get_pixel(mouse_x, mouse_y);
    
    // Draw the white cursor
    vga_put_pixel(mouse_x, mouse_y, CURSOR_COLOR);

    prev_x = mouse_x;
    prev_y = mouse_y;
}

void mouse_install() {
    mouse_wait_write();
    outb(MOUSE_STATUS_PORT, 0xA8);

    mouse_wait_write();
    outb(MOUSE_STATUS_PORT, 0x20); 

    mouse_wait_read();
    uint8_t status = (inb(MOUSE_DATA_PORT) | 0x02) & ~0x20;

    mouse_wait_write();
    outb(MOUSE_STATUS_PORT, 0x60);

    mouse_wait_write();
    outb(MOUSE_DATA_PORT, status);
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
    irq_install_handler(12, mouse_handler);
}

void mouse_set_drawing_mode(int active) {
    drawing_mode_active = active;
    right_clicked = 0;
}

int mouse_right_clicked() { return right_clicked; }
int mouse_get_x()         { return mouse_x; }
int mouse_get_y()         { return mouse_y; }
int mouse_left_held()     { return left_button; }