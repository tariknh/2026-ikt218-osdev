#include "scroll.h"
#include "terminal.h"
#include <libc/stdbool.h>
#include <libc/stddef.h>

// Text size for the banner
#define SCROLL_TEXT_MAX    256
// Speed of the banner movement
#define SCROLL_STEP_TICKS  155u /* ~18 chars/sec at PIT 1000Hz 
higher value = slower
lower value = faster
*/

// Banner text and animation state
static char scroll_text[SCROLL_TEXT_MAX];
static uint32_t scroll_len = 0;
static uint32_t scroll_offset = 0;
static uint32_t next_tick = 0;
static bool scroll_active = false;
// Simple color cycle for the banner
static const uint8_t scroll_colors[] = {
    0x0C, 0x0E, 0x0A, 0x0B, 0x09, 0x0D, 0x0F
};

// Count text length with a maximum limit
static uint32_t strn_len(const char *s, uint32_t max) {
    uint32_t n = 0;
    while (n < max && s[n] != '\0') n++;
    return n;
}

// Draw the moving banner on the bottom row
static void draw_row(void) {
    int w = terminal_width();
    int h = terminal_height();
    int y = h - 1;

    if (!scroll_active || scroll_len == 0) {
        for (int x = 0; x < w; x++) {
            terminal_put_at(x, y, ' ');
        }
        return;
    }

    for (int x = 0; x < w; x++) {
        uint32_t idx = (scroll_offset + (uint32_t)x) % (scroll_len + 3u);
        char ch = (idx < scroll_len) ? scroll_text[idx] : ' ';
        uint8_t color = scroll_colors[(scroll_offset + (uint32_t)x) % (uint32_t)(sizeof(scroll_colors))];
        if (ch == ' ') {
            color = 0x0A;
        }
        terminal_put_colored_at(x, y, ch, color);
    }
}

// Start the scrolling banner
void scroll_start(const char *text) {
    if (!text) text = "LocOS";

    // Copy the text into the local buffer
    scroll_len = strn_len(text, SCROLL_TEXT_MAX - 1u);
    for (uint32_t i = 0; i < scroll_len; i++) {
        scroll_text[i] = text[i];
    }
    scroll_text[scroll_len] = '\0';

    scroll_offset = 0;
    next_tick = 0;
    scroll_active = true;

    // Reserve the last row for the banner
    terminal_reserve_bottom_rows(1);
    draw_row();
}

// Stop the scrolling banner
void scroll_stop(void) {
    scroll_active = false;
    draw_row();
    terminal_reserve_bottom_rows(0);
}

// Move the banner one step on each PIT tick
void scroll_on_tick(uint32_t tick) {
    if (!scroll_active || scroll_len == 0) {
        return;
    }
    if ((int32_t)(tick - next_tick) < 0) {
        return;
    }

    scroll_offset = (scroll_offset + 1u) % (scroll_len + 3u);
    next_tick = tick + SCROLL_STEP_TICKS;
    draw_row();
}