#include "piano.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "libc/stdint.h"

extern void sleep_interrupt(uint32_t ms);

#define PIT_BASE_FREQUENCY 1193180

#define PINK_BG       VGA_COLOR_LIGHT_MAGENTA
#define PINK_TEXT     VGA_COLOR_LIGHT_BROWN
#define GOLD_TEXT     VGA_COLOR_BROWN

#define FRAME_BG      VGA_COLOR_LIGHT_GREY
#define FRAME_FG      VGA_COLOR_BLACK

/* Press feedback */
#define WHITE_FG      VGA_COLOR_BLACK
#define WHITE_BG      VGA_COLOR_WHITE
#define WHITE_PRESS_FG  VGA_COLOR_BLACK
#define WHITE_PRESS_BG  VGA_COLOR_LIGHT_CYAN

#define BLACK_FG      VGA_COLOR_WHITE
#define BLACK_BG      VGA_COLOR_BLACK
#define BLACK_PRESS_FG  VGA_COLOR_WHITE
#define BLACK_PRESS_BG  VGA_COLOR_DARK_GREY

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

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

static void pcspk_play(uint32_t frequency) {
    if (frequency == 0) return;
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) outb(0x61, tmp | 3);
}

static void pcspk_stop(void) {
    uint8_t tmp = inb(0x61);
    outb(0x61, tmp & 0xFC);
}

static char white_label_from_scancode(uint8_t s) {
    switch (s) {
        case 0x02: case 0x1E: return '1';
        case 0x03: case 0x1F: return '2';
        case 0x04: case 0x20: return '3';
        case 0x05: case 0x21: return '4';
        case 0x06: case 0x22: return '5';
        case 0x07: case 0x23: return '6';
        case 0x08: case 0x24: return '7';
        case 0x09: case 0x25: return '8';
        case 0x26: return 'L';
        default: return 0;
    }
}

static char black_label_from_scancode(uint8_t s) {
    switch (s) {
        case 0x10: return 'Q'; case 0x11: return 'W';
        case 0x13: return 'R'; case 0x14: return 'T';
        case 0x15: return 'Y'; case 0x17: return 'I';
        case 0x18: return 'O';
        default: return 0;
    }
}

static uint32_t frequency_from_scancode(uint8_t s) {
    switch (s) {
        case 0x02: return 262;  case 0x03: return 294;
        case 0x04: return 330;  case 0x05: return 349;
        case 0x06: return 392;  case 0x07: return 440;
        case 0x08: return 494;  case 0x09: return 523;
        case 0x1E: return 262;  case 0x1F: return 294;
        case 0x20: return 330;  case 0x21: return 349;
        case 0x22: return 392;  case 0x23: return 440;
        case 0x24: return 494;  case 0x25: return 523;
        case 0x26: return 587;
        case 0x10: return 277;  case 0x11: return 311;
        case 0x13: return 370;  case 0x14: return 415;
        case 0x15: return 466;  case 0x17: return 554;
        case 0x18: return 622;
        case 0x2C: return 131;  case 0x2D: return 147;
        case 0x2E: return 165;  case 0x2F: return 175;
        case 0x30: return 196;  case 0x31: return 220;
        case 0x32: return 247;
        default: return 0;
    }
}

static const char* note_name_from_scancode(uint8_t s) {
    switch (s) {
        case 0x02: case 0x1E: return "C4 ";
        case 0x03: case 0x1F: return "D4 ";
        case 0x04: case 0x20: return "E4 ";
        case 0x05: case 0x21: return "F4 ";
        case 0x06: case 0x22: return "G4 ";
        case 0x07: case 0x23: return "A4 ";
        case 0x08: case 0x24: return "B4 ";
        case 0x09: case 0x25: return "C5 ";
        case 0x26: return "D5 ";
        case 0x10: return "C#4"; case 0x11: return "D#4";
        case 0x13: return "F#4"; case 0x14: return "G#4";
        case 0x15: return "A#4"; case 0x17: return "C#5";
        case 0x18: return "D#5";
        case 0x2C: return "C3 "; case 0x2D: return "D3 ";
        case 0x2E: return "E3 "; case 0x2F: return "F3 ";
        case 0x30: return "G3 "; case 0x31: return "A3 ";
        case 0x32: return "B3 ";
        default: return "-- ";
    }
}

static void draw_piano_screen(uint8_t pressed) {
    fillScreen(VGA_COLOR_WHITE, PINK_BG);

    /* HEADER */
    draw_text_center(1, "================================================",
                     GOLD_TEXT, PINK_BG);
    draw_text_center(2, "  M A R L E N   &   S A B I N A   P I A N O  ",
                     VGA_COLOR_WHITE, PINK_BG);
    draw_text_center(3, "================================================",
                     GOLD_TEXT, PINK_BG);

    /* Now playing */
    const char* note = note_name_from_scancode(pressed);
    draw_text(30, 5, "Now Playing: [ ", VGA_COLOR_WHITE, PINK_BG);
    draw_text(46, 5, note, GOLD_TEXT, PINK_BG);
    draw_text(49, 5, " ]", VGA_COLOR_WHITE, PINK_BG);

    /* === PIANO ===
     * 16 keys + 2 frame-color gap columns (one between E-F, one between B-C)
     * Each key = 3 wide, gap = 1 col of frame
     *
     * Sequence with gaps shown as '|':
     *   1 Q 2 W 3 | 4 R 5 T 6 Y 7 | 8 I L O
     *   W B W B W   W B W B W B W   W B W B
     *
     * 16 keys * 3 = 48, plus 2 gap cols = 50 total inner width
     * Add 2 frame side walls = 52 total. Centered: x = (80-52)/2 = 14.
     */

    uint16_t frame_x = 14;
    uint16_t frame_y = 7;
    uint16_t frame_w = 52;
    uint16_t frame_h = 9;
    uint16_t key_y   = 8;

    /* Frame: top + bottom rows + side walls */
    for (uint16_t x = frame_x; x < frame_x + frame_w; x++) {
        putCharAt(x, frame_y,                  ' ', FRAME_FG, FRAME_BG);
        putCharAt(x, frame_y + frame_h - 2,    ' ', FRAME_FG, FRAME_BG);
        putCharAt(x, frame_y + frame_h - 1,    ' ', FRAME_FG, FRAME_BG);
    }
    for (uint16_t y = frame_y; y < frame_y + frame_h; y++) {
        putCharAt(frame_x,                  y, ' ', FRAME_FG, FRAME_BG);
        putCharAt(frame_x + frame_w - 1,    y, ' ', FRAME_FG, FRAME_BG);
    }

    /* Key data
     * type: 1=white, 0=black, 2=GAP (frame color spacer) */
    int   types[18]  = { 1, 0, 1, 0, 1,  2,  1, 0, 1, 0, 1, 0, 1,  2,  1, 0, 1, 0 };
    char  labels[18] = {'1','Q','2','W','3', 0 ,'4','R','5','T','6','Y','7', 0 ,'8','I','L','O'};
    char  notes[18]  = {'C', 0 ,'D', 0 ,'E', 0 ,'F', 0 ,'G', 0 ,'A', 0 ,'B', 0 ,'C', 0 ,'D', 0 };
    int   widths[18] = { 3, 3, 3, 3, 3,  1,  3, 3, 3, 3, 3, 3, 3,  1,  3, 3, 3, 3 };

    char w_pressed = white_label_from_scancode(pressed);
    char b_pressed = black_label_from_scancode(pressed);

    /* Inner area starts 1 col after left frame wall */
    uint16_t kx = frame_x + 1;

    for (int i = 0; i < 18; i++) {
        uint16_t x = kx;
        uint16_t kw = widths[i];
        int type = types[i];

        if (type == 2) {
            /* Frame-colored gap, full key area height */
            for (uint16_t row = 0; row < 6; row++) {
                putCharAt(x, key_y + row, ' ', FRAME_FG, FRAME_BG);
            }
        } else {
            int is_white = (type == 1);
            int is_pressed = is_white
                ? (labels[i] == w_pressed)
                : (labels[i] == b_pressed);

            uint8_t fg, bg;
            uint16_t key_height;

            if (is_white) {
                fg = is_pressed ? WHITE_PRESS_FG : WHITE_FG;
                bg = is_pressed ? WHITE_PRESS_BG : WHITE_BG;
                key_height = 6;
            } else {
                fg = is_pressed ? BLACK_PRESS_FG : BLACK_FG;
                bg = is_pressed ? BLACK_PRESS_BG : BLACK_BG;
                key_height = 3;
            }

            /* Draw the key */
            for (uint16_t row = 0; row < key_height; row++) {
                for (uint16_t col = 0; col < kw; col++) {
                    putCharAt(x + col, key_y + row, ' ', fg, bg);
                }
            }

            /* Below short black keys: frame color (blends into bottom bar) */
            if (!is_white) {
                for (uint16_t row = key_height; row < 6; row++) {
                    for (uint16_t col = 0; col < kw; col++) {
                        putCharAt(x + col, key_y + row, ' ',
                                  FRAME_FG, FRAME_BG);
                    }
                }
            }

            /* Label near top */
            putCharAt(x + 1, key_y + 1, labels[i], fg, bg);

            /* Note name on white keys, near bottom */
            if (is_white && notes[i]) {
                uint8_t nfg = is_pressed
                    ? VGA_COLOR_BLACK
                    : VGA_COLOR_LIGHT_MAGENTA;
                putCharAt(x + 1, key_y + 4, notes[i], nfg, bg);
            }
        }

        kx += kw;
    }

    /* === CONTROLS === */
    draw_text_center(18, "----------------- CONTROLS -----------------",
                     VGA_COLOR_WHITE, PINK_BG);
    draw_text(20, 19, "White: ", GOLD_TEXT, PINK_BG);
    draw_text(27, 19, "1-8 L   or   A-K L", VGA_COLOR_WHITE, PINK_BG);
    draw_text(20, 20, "Black: ", GOLD_TEXT, PINK_BG);
    draw_text(27, 20, "Q W  R T Y  I O", VGA_COLOR_WHITE, PINK_BG);
    draw_text(20, 21, "Bass:  ", GOLD_TEXT, PINK_BG);
    draw_text(27, 21, "Z X C V B N M", VGA_COLOR_WHITE, PINK_BG);
    draw_text(20, 22, "Exit:  ", GOLD_TEXT, PINK_BG);
    draw_text(27, 22, "ESC", VGA_COLOR_WHITE, PINK_BG);

    draw_text_center(24, "* ~ In Pink We Trust ~ *",
                     PINK_TEXT, PINK_BG);
}

void run_piano(void) {
    keyboard_set_echo(false);
    draw_piano_screen(0);

    while (1) {
        uint8_t scancode = keyboard_get_scancode();
        if (scancode == 0x01) { pcspk_stop(); return; }

        uint32_t frequency = frequency_from_scancode(scancode);
        if (frequency != 0) {
            draw_piano_screen(scancode);
            pcspk_play(frequency);
            sleep_interrupt(160);
            pcspk_stop();
            draw_piano_screen(0);
        }
        sleep_interrupt(20);
    }
}