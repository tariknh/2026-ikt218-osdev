#include "libc/stdint.h"
#include "terminal.h"
#include "marlen_sabina_intro.h"

extern void sleep_interrupt(uint32_t ms);

/* ================================================================== */
/* Colour palette                                                      */
/* ================================================================== */

#define BG_PINK     VGA_COLOR_LIGHT_MAGENTA   
#define C_YELLOW    VGA_COLOR_LIGHT_BROWN     
#define C_ORANGE    VGA_COLOR_BROWN           
#define C_WHITE     VGA_COLOR_WHITE
#define C_DIM       VGA_COLOR_LIGHT_GREY

/* ================================================================== */
/* helpers                                                             */
/* ================================================================== */

static uint16_t str_len(const char* s)
{
    uint16_t n = 0;
    while (s[n]) n++;
    return n;
}

static void draw_text(uint16_t x, uint16_t y, const char* s,
                      uint8_t fg, uint8_t bg)
{
    for (uint16_t i = 0; s[i]; i++)
        putCharAt(x + i, y, s[i], fg, bg);
}

static void draw_text_center(uint16_t y, const char* s, uint8_t fg, uint8_t bg)
{
    uint16_t sw = getScreenWidth();
    uint16_t n  = str_len(s);
    if (n > sw) return;
    draw_text((sw - n) / 2, y, s, fg, bg);
}

static void draw_art(uint16_t x, uint16_t y, const char* const* lines,
                     uint16_t count, uint8_t fg, uint8_t bg)
{
    for (uint16_t r = 0; r < count; r++) {
        const char* line = lines[r];
        for (uint16_t c = 0; line[c]; c++) {
            if (line[c] != ' ')
                putCharAt(x + c, y + r, line[c], fg, bg);
        }
    }
}

static void clear_art(uint16_t x, uint16_t y, const char* const* lines,
                      uint16_t count, uint8_t bg)
{
    for (uint16_t r = 0; r < count; r++) {
        const char* line = lines[r];
        for (uint16_t c = 0; line[c]; c++) {
            putCharAt(x + c, y + r, ' ', C_WHITE, bg);
        }
    }
}

/* tiny random */
static uint32_t rng = 0xC0FFEE42;
static uint32_t rnd(void)
{
    rng = rng * 1103515245 + 12345;
    return (rng >> 16) & 0x7FFF;
}

/* ================================================================== */
/* fades                                                               */
/* ================================================================== */

static void fade_in(void)
{
    fillScreen(C_WHITE, VGA_COLOR_BLACK);          sleep_interrupt(180);
    fillScreen(C_WHITE, VGA_COLOR_DARK_GREY);      sleep_interrupt(140);
    fillScreen(C_WHITE, VGA_COLOR_MAGENTA);        sleep_interrupt(140);
    fillScreen(C_WHITE, BG_PINK);                  sleep_interrupt(180);
}

static void fade_out(void)
{
    fillScreen(C_WHITE, BG_PINK);                  sleep_interrupt(180);
    fillScreen(C_WHITE, VGA_COLOR_MAGENTA);        sleep_interrupt(140);
    fillScreen(C_WHITE, VGA_COLOR_DARK_GREY);      sleep_interrupt(140);
    fillScreen(C_WHITE, VGA_COLOR_BLACK);          sleep_interrupt(180);
}

/* ================================================================== */
/* ASCII art                                                           */
/* ================================================================== */

static const char* heart_big[] = {
    "   .d88b. .d88b.   ",
    "  d88888dd88888b   ",
    "  Y88888888888888P ",
    "   Y888888888888P  ",
    "    Y8888888888P   ",
    "      Y888888P     ",
    "        Y88P       ",
};

static const char* heart_pulse[] = {
    "  .d8888b. d8888b.   ",
    " d888888b8d888888b.  ",
    "d88888888888888888Yb ",
    "Y888888888888888888P ",
    " Y88888888888888P    ",
    "  Y888888888888P     ",
    "    Y8888888P        ",
    "       YP            ",
};

#define HEART_BIG_ROWS   7
#define HEART_PULSE_ROWS 8

static const char* heart_small[] = {
    " _ _ ",
    "(_V_)",
    " \\_/ ",
};
#define HEART_SMALL_ROWS 3

static const char* title_art[] = {
" __  __   _   ___  _    ___ _  _    _   ___   _   ___ ___ _  _   _   ",
"|  \\/  | /_\\ | _ \\| |  | __| \\| |  ( ) / __| /_\\ | _ )_ _| \\| | /_\\  ",
"| |\\/| |/ _ \\|   /| |__| _|| .` |  |/  \\__ \\/ _ \\| _ \\| || .` |/ _ \\ ",
"|_|  |_/_/ \\_\\_|_\\|____|___|_|\\_|      |___/_/ \\_\\___/___|_|\\_/_/ \\_\\",
};
#define TITLE_ROWS 4

/* ================================================================== */
/* ANIMATION 1: hearts float up from bottom                            */
/* ================================================================== */

static void hearts_float_up(uint8_t bg)
{
    uint16_t sw = getScreenWidth();
    uint16_t sh = getScreenHeight();

    int16_t hx[6];
    int16_t hy[6];
    uint8_t hcolor[6];

    for (int i = 0; i < 6; i++) {
        hx[i] = (int16_t)((sw / 7) * (i + 1) - 2);
        hy[i] = (int16_t)(sh + (rnd() % 6));
        /* Alternate yellow and orange hearts */
        hcolor[i] = (i % 2 == 0) ? C_YELLOW : C_ORANGE;
    }

    for (int frame = 0; frame < 30; frame++) {
        for (int i = 0; i < 6; i++) {
            if (hy[i] >= 0 && hy[i] < (int16_t)sh) {
                clear_art((uint16_t)hx[i], (uint16_t)hy[i],
                          heart_small, HEART_SMALL_ROWS, bg);
            }
            hy[i] -= 1;
            if (hy[i] >= 0 && hy[i] < (int16_t)sh - HEART_SMALL_ROWS + 1) {
                draw_art((uint16_t)hx[i], (uint16_t)hy[i],
                         heart_small, HEART_SMALL_ROWS, hcolor[i], bg);
            }
        }
        sleep_interrupt(90);
    }

    fillScreen(C_WHITE, bg);
}

/* ================================================================== */
/* ANIMATION 2: title types in letter by letter (YELLOW)               */
/* ================================================================== */

static void title_typewriter(uint8_t bg)
{
    uint16_t sw = getScreenWidth();
    uint16_t title_w = str_len(title_art[0]);
    uint16_t tx = (sw - title_w) / 2;
    uint16_t ty = 3;

    for (uint16_t c = 0; c < title_w; c++) {
        for (uint16_t r = 0; r < TITLE_ROWS; r++) {
            char ch = title_art[r][c];
            if (ch != ' ' && ch != '\0') {
                putCharAt(tx + c, ty + r, ch, C_YELLOW, bg);
            }
        }
        sleep_interrupt(15);
    }
}

/* ================================================================== */
/* ANIMATION 3: big heart beats (ORANGE)                               */
/* ================================================================== */

static void heart_beat(uint8_t bg, int beats)
{
    uint16_t sw = getScreenWidth();

    uint16_t big_w   = str_len(heart_big[0]);
    uint16_t pulse_w = str_len(heart_pulse[0]);

    uint16_t big_x   = (sw - big_w)   / 2;
    uint16_t pulse_x = (sw - pulse_w) / 2;
    uint16_t hy = 10;

    for (int b = 0; b < beats; b++) {
        for (uint16_t r = 0; r < HEART_PULSE_ROWS; r++) {
            for (uint16_t c = 0; c < pulse_w; c++) {
                putCharAt(pulse_x + c, hy + r, ' ', C_WHITE, bg);
            }
        }

        /* normal = orange */
        draw_art(big_x, hy, heart_big, HEART_BIG_ROWS, VGA_COLOR_RED, bg);
        sleep_interrupt(400);

        for (uint16_t r = 0; r < HEART_BIG_ROWS; r++) {
            for (uint16_t c = 0; c < big_w; c++) {
                putCharAt(big_x + c, hy + r, ' ', C_WHITE, bg);
            }
        }

        /* pulsed = yellow (brighter, feels like a beat) */
        draw_art(pulse_x, hy, heart_pulse, HEART_PULSE_ROWS, C_YELLOW, bg);
        sleep_interrupt(220);

        for (uint16_t r = 0; r < HEART_PULSE_ROWS; r++) {
            for (uint16_t c = 0; c < pulse_w; c++) {
                putCharAt(pulse_x + c, hy + r, ' ', C_WHITE, bg);
            }
        }

        draw_art(big_x, hy, heart_big, HEART_BIG_ROWS, VGA_COLOR_RED, bg);
        sleep_interrupt(300);
    }
}

/* ================================================================== */
/* ANIMATION 4: sparkle bloom                                          */
/* ================================================================== */

static void sparkle_bloom(uint8_t bg, uint32_t frames)
{
    uint16_t sw = getScreenWidth();
    uint16_t sh = getScreenHeight();

    for (uint32_t f = 0; f < frames; f++) {
        for (int i = 0; i < 5; i++) {
            uint16_t x = rnd() % sw;
            uint16_t y = rnd() % sh;

            if (y >= 3 && y <= 7 && x >= 4 && x <= 75) continue;
            if (y >= 9 && y <= 17 && x >= 28 && x <= 52) continue;
            if (y >= 19 && y <= 22) continue;

            char c = "*.+"[rnd() % 3];
            /* Alternate white, yellow, orange sparkles */
            uint8_t color;
            uint32_t r = rnd() % 3;
            if      (r == 0) color = C_WHITE;
            else if (r == 1) color = C_YELLOW;
            else             color = C_ORANGE;

            putCharAt(x, y, c, color, bg);
        }

        sleep_interrupt(120);

        if (f % 2 == 0) {
            for (int i = 0; i < 4; i++) {
                uint16_t x = rnd() % sw;
                uint16_t y = rnd() % sh;
                if (y >= 3 && y <= 7 && x >= 4 && x <= 75) continue;
                if (y >= 9 && y <= 17 && x >= 28 && x <= 52) continue;
                if (y >= 19 && y <= 22) continue;
                putCharAt(x, y, ' ', C_WHITE, bg);
            }
        }
    }
}

/* ================================================================== */
/* ANIMATION 5: tagline with yellow hearts                             */
/* ================================================================== */

static void tagline_wipe(uint8_t bg)
{
    uint16_t sw = getScreenWidth();

    const char* tag = "In Pink We Trust";
    uint16_t tl = str_len(tag);
    uint16_t tx = (sw - tl - 4) / 2;
    uint16_t ty = 20;

    /* Hearts in yellow, text in orange */
    putCharAt(tx, ty, 3, C_YELLOW, bg);
    sleep_interrupt(150);
    putCharAt(tx + 3 + tl, ty, 3, C_YELLOW, bg);
    sleep_interrupt(150);

    for (uint16_t i = 0; i < tl; i++) {
        putCharAt(tx + 2 + i, ty, tag[i], C_ORANGE, bg);
        sleep_interrupt(60);
    }
}

/* ================================================================== */
/* main                                                                */
/* ================================================================== */

void marlen_sabina_intro(void)
{
    uint8_t pink = BG_PINK;

    fade_in();

    hearts_float_up(pink);

    title_typewriter(pink);

    sleep_interrupt(300);

    heart_beat(pink, 3);

    tagline_wipe(pink);

    sparkle_bloom(pink, 15);

    sleep_interrupt(800);

    fade_out();

    terminal_initialize();
}