/*
 * notes.c - Love Notes Text Editor
 *
 * A simple in-memory text editor with a pink aesthetic.
 * - Type to write (uses your keyboard's existing ASCII translation)
 * - Caps Lock toggles uppercase
 * - Backspace to delete
 * - Enter for new line
 * - Arrow keys to move cursor
 * - F1 to clear all (with confirmation)
 * - ESC to return to menu
 *
 * Notes are kept in a static buffer — they persist across visits
 * within the same boot session.
 */

#include "notes.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "libc/stdint.h"

extern void sleep_interrupt(uint32_t ms);

/* ================================================================== */
/* Constants                                                           */
/* ================================================================== */

#define SC_ESC      0x01
#define SC_BACKSP   0x0E
#define SC_ENTER    0x1C
#define SC_CAPS     0x3A
#define SC_F1       0x3B
#define SC_UP       0x48
#define SC_DOWN     0x50
#define SC_LEFT     0x4B
#define SC_RIGHT    0x4D
#define SC_TAB      0x0F

#define PINK_BG       VGA_COLOR_LIGHT_MAGENTA
#define PINK_TEXT     VGA_COLOR_LIGHT_BROWN
#define GOLD_TEXT     VGA_COLOR_BROWN

/* Editor area */
#define EDIT_X        4
#define EDIT_Y        5
#define EDIT_W        72
#define EDIT_H        16

/* Paper colors */
#define PAPER_FG      VGA_COLOR_BLACK
#define PAPER_BG      VGA_COLOR_WHITE

/* Cursor block */
#define CURSOR_FG     VGA_COLOR_WHITE
#define CURSOR_BG     VGA_COLOR_LIGHT_RED

/* ================================================================== */
/* Buffer                                                              */
/* ================================================================== */

#define BUF_ROWS  EDIT_H
#define BUF_COLS  EDIT_W

static char buffer[BUF_ROWS][BUF_COLS];

static uint16_t cur_row = 0;
static uint16_t cur_col = 0;

/* Caps Lock state — toggled by user */
static uint8_t caps_lock = 0;

/* ================================================================== */
/* Helpers                                                             */
/* ================================================================== */

static uint16_t slen(const char* s) {
    uint16_t n = 0;
    while (s[n]) n++;
    return n;
}

static void draw_text(uint16_t x, uint16_t y, const char* s,
                      uint8_t fg, uint8_t bg) {
    for (uint16_t i = 0; s[i]; i++) putCharAt(x + i, y, s[i], fg, bg);
}

static void draw_text_center(uint16_t y, const char* s,
                             uint8_t fg, uint8_t bg) {
    uint16_t len = slen(s);
    draw_text((80 - len) / 2, y, s, fg, bg);
}

/* ================================================================== */
/* Drawing                                                             */
/* ================================================================== */

static void draw_header(void) {
    draw_text_center(1, "================================================",
                     GOLD_TEXT, PINK_BG);
    draw_text_center(2, "          ~  L O V E   N O T E S  ~          ",
                     VGA_COLOR_WHITE, PINK_BG);
    draw_text_center(3, "================================================",
                     GOLD_TEXT, PINK_BG);
}

static void draw_status(void) {
    /* Clear status line */
    for (uint16_t x = 0; x < 80; x++) {
        putCharAt(x, 4, ' ', VGA_COLOR_WHITE, PINK_BG);
    }
    if (caps_lock) {
        draw_text(4, 4, "[ CAPS ]", GOLD_TEXT, PINK_BG);
    }
}

static void draw_footer(void) {
    for (uint16_t x = 0; x < 80; x++) {
        putCharAt(x, 22, ' ', VGA_COLOR_WHITE, PINK_BG);
        putCharAt(x, 23, ' ', VGA_COLOR_WHITE, PINK_BG);
        putCharAt(x, 24, ' ', VGA_COLOR_WHITE, PINK_BG);
    }

    draw_text_center(22,
        "Type to write  *  Backspace to delete  *  Arrows to move",
        VGA_COLOR_WHITE, PINK_BG);
    draw_text_center(23,
        "Caps Lock = uppercase  *  F1 = clear  *  ESC = back",
        PINK_TEXT, PINK_BG);
    draw_text_center(24, "* ~ Write what's in your heart ~ *",
                     PINK_TEXT, PINK_BG);
}

static void draw_paper(void) {
    /* Frame around the paper */
    for (uint16_t y = EDIT_Y - 1; y < EDIT_Y + EDIT_H + 1; y++) {
        putCharAt(EDIT_X - 1,        y, ' ', VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
        putCharAt(EDIT_X + EDIT_W,   y, ' ', VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    }
    for (uint16_t x = EDIT_X - 1; x < EDIT_X + EDIT_W + 1; x++) {
        putCharAt(x, EDIT_Y - 1,         ' ', VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
        putCharAt(x, EDIT_Y + EDIT_H,    ' ', VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    }

    /* Paper content */
    for (uint16_t y = 0; y < EDIT_H; y++) {
        for (uint16_t x = 0; x < EDIT_W; x++) {
            char ch = buffer[y][x];
            if (ch == 0) ch = ' ';
            putCharAt(EDIT_X + x, EDIT_Y + y, ch, PAPER_FG, PAPER_BG);
        }
    }
}

static void redraw_cell(uint16_t row, uint16_t col) {
    char ch = buffer[row][col];
    if (ch == 0) ch = ' ';
    putCharAt(EDIT_X + col, EDIT_Y + row, ch, PAPER_FG, PAPER_BG);
}

static void draw_cursor(void) {
    char ch = buffer[cur_row][cur_col];
    if (ch == 0) ch = ' ';
    putCharAt(EDIT_X + cur_col, EDIT_Y + cur_row, ch, CURSOR_FG, CURSOR_BG);
}

static void erase_cursor(void) {
    redraw_cell(cur_row, cur_col);
}

/* ================================================================== */
/* Editor actions                                                      */
/* ================================================================== */

static void clear_buffer(void) {
    for (uint16_t r = 0; r < BUF_ROWS; r++) {
        for (uint16_t c = 0; c < BUF_COLS; c++) {
            buffer[r][c] = 0;
        }
    }
    cur_row = 0;
    cur_col = 0;
}

static char to_upper(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

static void insert_char(char ch) {
    if (cur_col >= BUF_COLS) return;

    if (caps_lock) ch = to_upper(ch);

    buffer[cur_row][cur_col] = ch;
    redraw_cell(cur_row, cur_col);

    cur_col++;
    if (cur_col >= BUF_COLS) {
        if (cur_row < BUF_ROWS - 1) {
            cur_row++;
            cur_col = 0;
        } else {
            cur_col = BUF_COLS - 1;
        }
    }
}

static void backspace(void) {
    if (cur_col > 0) {
        cur_col--;
        buffer[cur_row][cur_col] = 0;
        redraw_cell(cur_row, cur_col);
    } else if (cur_row > 0) {
        cur_row--;
        int16_t c = BUF_COLS - 1;
        while (c >= 0 && buffer[cur_row][c] == 0) c--;
        cur_col = (uint16_t)(c + 1);
        if (cur_col >= BUF_COLS) cur_col = BUF_COLS - 1;
    }
}

static void newline(void) {
    if (cur_row < BUF_ROWS - 1) {
        cur_row++;
        cur_col = 0;
    }
}

static void move_left(void) {
    if (cur_col > 0) cur_col--;
    else if (cur_row > 0) {
        cur_row--;
        cur_col = BUF_COLS - 1;
    }
}

static void move_right(void) {
    if (cur_col < BUF_COLS - 1) cur_col++;
    else if (cur_row < BUF_ROWS - 1) {
        cur_row++;
        cur_col = 0;
    }
}

static void move_up(void) {
    if (cur_row > 0) cur_row--;
}

static void move_down(void) {
    if (cur_row < BUF_ROWS - 1) cur_row++;
}

/* ================================================================== */
/* Confirm-clear dialog                                                */
/* ================================================================== */

static int confirm_clear(void) {
    uint16_t bx = 25;
    uint16_t by = 10;
    uint16_t bw = 30;
    uint16_t bh = 6;

    for (uint16_t y = 0; y < bh; y++) {
        for (uint16_t x = 0; x < bw; x++) {
            putCharAt(bx + x, by + y, ' ',
                      VGA_COLOR_BLACK, VGA_COLOR_WHITE);
        }
    }
    for (uint16_t x = 0; x < bw; x++) {
        putCharAt(bx + x, by,          ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
        putCharAt(bx + x, by + bh - 1, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
    }
    for (uint16_t y = 0; y < bh; y++) {
        putCharAt(bx,          by + y, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
        putCharAt(bx + bw - 1, by + y, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
    }

    draw_text_center(by + 2, "Clear all notes?",
                     VGA_COLOR_LIGHT_RED, VGA_COLOR_WHITE);
    draw_text_center(by + 4, "Y = yes    N = no",
                     VGA_COLOR_BLACK, VGA_COLOR_WHITE);

    while (1) {
        char c = keyboard_get_char();
        if (c == 'y' || c == 'Y') return 1;
        if (c == 'n' || c == 'N') return 0;

        uint8_t sc = keyboard_get_scancode();
        if (sc == SC_ESC) return 0;

        sleep_interrupt(30);
    }
}

/* ================================================================== */
/* Main editor loop                                                    */
/* ================================================================== */

void run_notes(void) {
    keyboard_set_echo(false);
    caps_lock = 0;

    fillScreen(VGA_COLOR_WHITE, PINK_BG);
    draw_header();
    draw_status();
    draw_paper();
    draw_footer();
    draw_cursor();

    while (1) {
        uint8_t sc = keyboard_get_scancode();

        /* Special keys (handled by scancode) */
        if (sc == SC_ESC) return;

        if (sc == SC_F1) {
            if (confirm_clear()) {
                clear_buffer();
            }
            /* Redraw whole UI */
            fillScreen(VGA_COLOR_WHITE, PINK_BG);
            draw_header();
            draw_status();
            draw_paper();
            draw_footer();
            draw_cursor();
            /* Drain any pending char from the dialog */
            (void)keyboard_get_char();
            continue;
        }

        if (sc == SC_CAPS) {
            caps_lock = !caps_lock;
            draw_status();
            /* The CAPS scancode also produced a char (none in your map) */
            (void)keyboard_get_char();
            continue;
        }

        if (sc == SC_UP || sc == SC_DOWN ||
            sc == SC_LEFT || sc == SC_RIGHT)
        {
            erase_cursor();
            switch (sc) {
                case SC_UP:    move_up();    break;
                case SC_DOWN:  move_down();  break;
                case SC_LEFT:  move_left();  break;
                case SC_RIGHT: move_right(); break;
            }
            draw_cursor();
            (void)keyboard_get_char();
            continue;
        }

        /* Printable characters via the keyboard's ASCII translation */
        char c = keyboard_get_char();

        if (c == '\b') {
            erase_cursor();
            backspace();
            draw_cursor();
        } else if (c == '\n') {
            erase_cursor();
            newline();
            draw_cursor();
        } else if (c == '\t') {
            /* Tab = 4 spaces */
            for (int i = 0; i < 4; i++) {
                erase_cursor();
                insert_char(' ');
                draw_cursor();
            }
        } else if (c >= 32 && c < 127) {
            erase_cursor();
            insert_char(c);
            draw_cursor();
        }

        sleep_interrupt(15);
    }
}