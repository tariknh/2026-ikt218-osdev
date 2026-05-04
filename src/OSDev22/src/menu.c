/*
 * menu.c - Pink Queen Menu for Assignment 6
 */

#include "menu.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "piano.h"
#include "snake.h"
#include "notes.h"
#include "pong.h"

extern void sleep_interrupt(uint32_t ms);

/* Scancodes we care about */
#define SC_UP     0x48
#define SC_DOWN   0x50
#define SC_ENTER  0x1C
#define SC_ESC    0x01

#define BG_PINK        VGA_COLOR_LIGHT_MAGENTA
#define BOX_BORDER     VGA_COLOR_WHITE
#define TEXT_NORMAL    VGA_COLOR_WHITE
#define TEXT_DIM       VGA_COLOR_LIGHT_GREY
#define TEXT_HOT_PINK  VGA_COLOR_LIGHT_BROWN
#define TEXT_GOLD      VGA_COLOR_BROWN
#define SEL_BG         VGA_COLOR_WHITE
#define SEL_FG         VGA_COLOR_BROWN

#define BOX_W  50
#define BOX_H  15
#define BOX_X  ((80 - BOX_W) / 2)
#define BOX_Y  5

/* ================================================================== */
/* Menu option data                                                    */
/* ================================================================== */

static const char* menu_labels[MENU_OPT_COUNT] = {
    "Pong Game",
    "Piano Keyboard",
    "Snake Game",
    "Love Notes",
    "About",
    "Exit",
};

static uint8_t selected = 0;

/* ================================================================== */
/* Tiny RNG for sparkles                                               */
/* ================================================================== */

static uint32_t rng = 0xBABE1234;
static uint32_t rnd(void)
{
    rng = rng * 1103515245 + 12345;
    return (rng >> 16) & 0x7FFF;
}

/* ================================================================== */
/* String helpers                                                      */
/* ================================================================== */

static uint16_t slen(const char* s)
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

static void draw_text_center(uint16_t y, const char* s,
                             uint8_t fg, uint8_t bg)
{
    uint16_t n = slen(s);
    if (n > 80) return;
    draw_text((80 - n) / 2, y, s, fg, bg);
}

/* ================================================================== */
/* Box drawing                                                         */
/* ================================================================== */

static void draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     uint8_t fg, uint8_t bg)
{
    putCharAt(x,         y,         201, fg, bg);
    putCharAt(x + w - 1, y,         187, fg, bg);
    putCharAt(x,         y + h - 1, 200, fg, bg);
    putCharAt(x + w - 1, y + h - 1, 188, fg, bg);

    for (uint16_t i = 1; i < w - 1; i++) {
        putCharAt(x + i, y,         205, fg, bg);
        putCharAt(x + i, y + h - 1, 205, fg, bg);
    }

    for (uint16_t i = 1; i < h - 1; i++) {
        putCharAt(x,         y + i, 186, fg, bg);
        putCharAt(x + w - 1, y + i, 186, fg, bg);
    }
}

static void fill_area(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                      uint8_t fg, uint8_t bg)
{
    for (uint16_t yy = 0; yy < h; yy++)
        for (uint16_t xx = 0; xx < w; xx++)
            putCharAt(x + xx, y + yy, ' ', fg, bg);
}

/* ================================================================== */
/* Title banner                                                        */
/* ================================================================== */

static void draw_banner(void)
{
    draw_text_center(1, ". * ~ . * ~ MARLEN & SABINA ~ * . ~ * .",
                     TEXT_HOT_PINK, BG_PINK);

    const char* crown = "  <>--<>  OS Dev 22  <>--<>  ";
    draw_text_center(2, crown, TEXT_GOLD, BG_PINK);

    uint16_t sw = 80;
    const char* hearts = " * . * . * . * . * . * . * ";
    uint16_t hlen = slen(hearts);
    draw_text((sw - hlen) / 2, 3, hearts, TEXT_NORMAL, BG_PINK);
}

/* ================================================================== */
/* Menu options                                                        */
/* ================================================================== */

static void draw_options(void)
{
    uint16_t inner_x = BOX_X + 2;
    uint16_t inner_y = BOX_Y + 2;
    uint16_t inner_w = BOX_W - 4;

    draw_text_center(BOX_Y + 1, "~ Choose Your Adventure ~",
                     TEXT_HOT_PINK, BG_PINK);

    for (uint8_t i = 0; i < MENU_OPT_COUNT; i++) {
        uint16_t row = inner_y + 1 + i * 2;

        if (i == selected) {
            for (uint16_t x = 0; x < inner_w; x++)
                putCharAt(inner_x + x, row, ' ', SEL_FG, SEL_BG);

            putCharAt(inner_x + 1, row, 3, SEL_FG, SEL_BG);
            putCharAt(inner_x + 2, row, ' ', SEL_FG, SEL_BG);

            draw_text(inner_x + 3, row, menu_labels[i], SEL_FG, SEL_BG);

            uint16_t label_len = slen(menu_labels[i]);
            putCharAt(inner_x + 3 + label_len + 1, row, 3, SEL_FG, SEL_BG);
        } else {
            for (uint16_t x = 0; x < inner_w; x++)
                putCharAt(inner_x + x, row, ' ', TEXT_NORMAL, BG_PINK);

            putCharAt(inner_x + 2, row, 249, TEXT_HOT_PINK, BG_PINK);

            draw_text(inner_x + 4, row, menu_labels[i], TEXT_NORMAL, BG_PINK);
        }
    }
}

/* ================================================================== */
/* Footer                                                              */
/* ================================================================== */

static void draw_footer(void)
{
    draw_text_center(22, "UP / DOWN to navigate  *  ENTER to select",
                     TEXT_NORMAL, BG_PINK);
    draw_text_center(23, "~ , ~",
                     TEXT_HOT_PINK, BG_PINK);
}

/* ================================================================== */
/* Sparkle animation                                                   */
/* ================================================================== */

static void animate_sparkles(void)
{
    for (int i = 0; i < 4; i++) {
        uint16_t x = rnd() % 80;
        uint16_t y = rnd() % 25;

        if (y <= 3) {
            if (x >= 15 && x <= 64) continue;
        }

        if (y >= BOX_Y && y < BOX_Y + BOX_H &&
            x >= BOX_X - 1 && x < BOX_X + BOX_W + 1) continue;

        if (y >= 22) continue;

        char c;
        uint32_t r = rnd() % 3;
        if      (r == 0) c = '*';
        else if (r == 1) c = 249;
        else             c = '.';

        putCharAt(x, y, c, TEXT_NORMAL, BG_PINK);
    }

    for (int i = 0; i < 3; i++) {
        uint16_t x = rnd() % 80;
        uint16_t y = rnd() % 25;

        if (y <= 3) {
            if (x >= 15 && x <= 64) continue;
        }
        if (y >= BOX_Y && y < BOX_Y + BOX_H &&
            x >= BOX_X - 1 && x < BOX_X + BOX_W + 1) continue;
        if (y >= 22) continue;

        putCharAt(x, y, ' ', TEXT_NORMAL, BG_PINK);
    }
}

/* ================================================================== */
/* Draw full menu screen                                               */
/* ================================================================== */

static void draw_menu(void)
{
    fillScreen(TEXT_NORMAL, BG_PINK);
    draw_banner();
    draw_box(BOX_X, BOX_Y, BOX_W, BOX_H, BOX_BORDER, BG_PINK);
    draw_options();
    draw_footer();
}

/* ================================================================== */
/* Menu-open slide animation                                           */
/* ================================================================== */

static void animate_open(void)
{
    fillScreen(TEXT_NORMAL, BG_PINK);
    draw_banner();
    draw_footer();

    uint16_t min_w = 8;
    uint16_t min_h = 3;
    uint16_t steps = 14;

    for (uint16_t s = 0; s <= steps; s++) {
        uint16_t w = min_w + (BOX_W - min_w) * s / steps;
        uint16_t h = min_h + (BOX_H - min_h) * s / steps;
        uint16_t x = (80 - w) / 2;
        uint16_t y = BOX_Y + (BOX_H - h) / 2;

        uint16_t cx = (80 - BOX_W) / 2 - 1;
        uint16_t cy = BOX_Y - 1;
        if (cy + BOX_H + 2 <= 25 && cx + BOX_W + 2 <= 80) {
            fill_area(cx, cy, BOX_W + 2, BOX_H + 2,
                      TEXT_NORMAL, BG_PINK);
        }

        draw_box(x, y, w, h, BOX_BORDER, BG_PINK);
        sleep_interrupt(30);
    }

    draw_menu();
}

/* ================================================================== */
/* Wipe transition                                                     */
/* ================================================================== */

static void wipe_out(void)
{
    fillScreen(VGA_COLOR_WHITE, VGA_COLOR_LIGHT_MAGENTA);
    sleep_interrupt(150);
    fillScreen(VGA_COLOR_WHITE, VGA_COLOR_MAGENTA);
    sleep_interrupt(150);
    fillScreen(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    sleep_interrupt(150);
}

/* ================================================================== */
/* Feature launchers                                                   */
/* ================================================================== */

static void wait_for_esc(void)
{
    while (1) {
        uint8_t sc = keyboard_get_scancode();
        if (sc == SC_ESC) return;
        sleep_interrupt(50);
    }
}

static void launch_pong(void)
{
    run_pong();
}

static void launch_piano(void)
{
    run_piano();
}

static void launch_snake(void)
{
    run_snake();
}

static void launch_notes(void)
{
    run_notes();
}

/* ================================================================== */
/* About screen                                                        */
/* ================================================================== */

/*
 * Replacement for show_about() in menu.c
 * Drop this in to replace the existing show_about function.
 */

void show_about(void)
{
    fillScreen(TEXT_NORMAL, BG_PINK);

    /* === Top decoration === */
    draw_text_center(0, ". * ~ * . * ~ * . * ~ * . * ~ * . * ~ * .",
                     TEXT_HOT_PINK, BG_PINK);

    /* === Title block with double border === */
    draw_text_center(2, "+============================================+",
                     TEXT_GOLD, BG_PINK);
    draw_text_center(3, "|                                            |",
                     TEXT_GOLD, BG_PINK);
    draw_text_center(4, "|              A  B  O  U  T                 |",
                     TEXT_NORMAL, BG_PINK);
    draw_text_center(5, "|                                            |",
                     TEXT_GOLD, BG_PINK);
    draw_text_center(6, "+============================================+",
                     TEXT_GOLD, BG_PINK);

    /* === OS Identity === */
    draw_text_center(8, "M   &   S    O S",
                     TEXT_HOT_PINK, BG_PINK);
    draw_text_center(9, "v1.0  -  Built with love",
                     TEXT_GOLD, BG_PINK);

    /* === Two-column layout for tech + credits === */

    /* Section divider */
    draw_text_center(11, "- - - - - - - - - - - - - - - - - - - - - - -",
                     TEXT_HOT_PINK, BG_PINK);

    /* Left column: tech stack */
    draw_text(15, 12, "  T E C H  ",  TEXT_GOLD,    BG_PINK);
    draw_text(15, 13, "i386 kernel",  TEXT_NORMAL,  BG_PINK);
    draw_text(15, 14, "PIT timer",    TEXT_NORMAL,  BG_PINK);
    draw_text(15, 15, "Paging + IRQ", TEXT_NORMAL,  BG_PINK);
    draw_text(15, 16, "VGA text mode",TEXT_NORMAL,  BG_PINK);
    draw_text(15, 17, "PC speaker",   TEXT_NORMAL,  BG_PINK);

    /* Right column: features */
    draw_text(48, 12, " F E A T U R E S ", TEXT_GOLD,   BG_PINK);
    draw_text(48, 13, "Pong  vs  AI",      TEXT_NORMAL, BG_PINK);
    draw_text(48, 14, "Piano keyboard",    TEXT_NORMAL, BG_PINK);
    draw_text(48, 15, "Snake game",        TEXT_NORMAL, BG_PINK);
    draw_text(48, 16, "Love notes",        TEXT_NORMAL, BG_PINK);
    draw_text(48, 17, "Pink everything",   TEXT_NORMAL, BG_PINK);

    draw_text_center(18, "- - - - - - - - - - - - - - - - - - - - - - -",
                     TEXT_HOT_PINK, BG_PINK);

    /* === Signature block === */
    draw_text_center(20, "~ In Pink We Trust ~", TEXT_HOT_PINK, BG_PINK);
    draw_text_center(21, "University of Agder  *  IKT218 OSDev 2026",
                     TEXT_GOLD, BG_PINK);

    /* === Footer === */
    draw_text_center(23, ". * ~ * . * ~ * . * ~ * . * ~ * . * ~ * .",
                     TEXT_HOT_PINK, BG_PINK);
    draw_text_center(24, "Press ESC to return",
                     TEXT_NORMAL, BG_PINK);

    wait_for_esc();
}
/* ================================================================== */
/* Main menu loop                                                      */
/* ================================================================== */

static void handle_enter(void)
{
    wipe_out();

    switch (selected) {
        case MENU_OPT_PONG:  launch_pong();  break;
        case MENU_OPT_PIANO: launch_piano(); break;
        case MENU_OPT_SNAKE: launch_snake(); break;
        case MENU_OPT_NOTES: launch_notes(); break;
        case MENU_OPT_ABOUT: show_about();   break;
        case MENU_OPT_EXIT:
            fillScreen(TEXT_NORMAL, BG_PINK);
            draw_text_center(11, "~ Goodbye, Queen ~",
                             TEXT_HOT_PINK, BG_PINK);
            draw_text_center(13, "Stay Pink",
                             TEXT_NORMAL, BG_PINK);
            sleep_interrupt(2000);
            fillScreen(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
            terminal_initialize();
            while (1) asm volatile ("hlt");
            return;
    }

    draw_menu();
}

void run_menu(void)
{
    keyboard_set_echo(false);

    animate_open();

    uint32_t sparkle_timer = 0;

    while (1) {
        uint8_t sc = keyboard_get_scancode();

        switch (sc) {
            case SC_UP:
                if (selected > 0) selected--;
                else              selected = MENU_OPT_COUNT - 1;
                draw_options();
                break;

            case SC_DOWN:
                if (selected < MENU_OPT_COUNT - 1) selected++;
                else                               selected = 0;
                draw_options();
                break;

            case SC_ENTER:
                handle_enter();
                break;

            default:
                break;
        }

        sparkle_timer++;
        if (sparkle_timer >= 3) {
            animate_sparkles();
            sparkle_timer = 0;
        }

        sleep_interrupt(80);
    }
}