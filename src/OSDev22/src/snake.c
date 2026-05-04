/*
 * snake.c - Pink Snake Game for Marlen & Sabina OS
 *
 * Classic snake game with a pink aesthetic.
 * - Arrow keys to move
 * - Eat the heart food to grow
 * - Don't hit the walls or yourself
 * - ESC returns to menu
 */

#include "snake.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "libc/stdint.h"

extern void sleep_interrupt(uint32_t ms);

/* ================================================================== */
/* Constants                                                           */
/* ================================================================== */

#define SC_UP     0x48
#define SC_DOWN   0x50
#define SC_LEFT   0x4B
#define SC_RIGHT  0x4D
#define SC_ESC    0x01
#define SC_SPACE  0x39
#define SC_ENTER  0x1C

#define PINK_BG       VGA_COLOR_LIGHT_MAGENTA
#define PINK_TEXT     VGA_COLOR_LIGHT_BROWN
#define GOLD_TEXT     VGA_COLOR_BROWN

/* Game board — leave room for header (4 rows) and footer (2 rows) */
#define BOARD_X       2
#define BOARD_Y       5
#define BOARD_W       76
#define BOARD_H       18

/* Snake colors */
#define SNAKE_HEAD_FG    VGA_COLOR_WHITE
#define SNAKE_HEAD_BG    VGA_COLOR_LIGHT_RED
#define SNAKE_BODY_FG    VGA_COLOR_WHITE
#define SNAKE_BODY_BG    VGA_COLOR_RED

/* Food */
#define FOOD_FG          VGA_COLOR_LIGHT_RED
#define FOOD_BG          PINK_BG

/* Wall */
#define WALL_FG          VGA_COLOR_BLACK
#define WALL_BG          VGA_COLOR_LIGHT_GREY

/* Snake direction */
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} direction_t;

/* Max snake length — enough for a full board */
#define MAX_SNAKE  256

typedef struct {
    int16_t x;
    int16_t y;
} point_t;

/* ================================================================== */
/* Game state                                                          */
/* ================================================================== */

static point_t   snake[MAX_SNAKE];
static uint16_t  snake_len;
static direction_t cur_dir;
static direction_t next_dir;
static point_t   food;
static uint32_t  score;
static uint32_t  high_score = 0;
static uint8_t   game_over;

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

/* Convert score (uint32) to string */
static void uint_to_str(uint32_t n, char* buf) {
    if (n == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    char tmp[16];
    int i = 0;
    while (n > 0) {
        tmp[i++] = '0' + (n % 10);
        n /= 10;
    }
    int j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = 0;
}

/* ================================================================== */
/* Pseudo-random number generator                                      */
/* ================================================================== */

static uint32_t rng_state = 0xCAFEBABE;

static uint32_t rnd(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

/* Churn the RNG state between games so each new game differs */
static void rng_advance_seed(void) {
    for (int i = 0; i < 7; i++) (void)rnd();
}

/* ================================================================== */
/* Drawing                                                             */
/* ================================================================== */

static void draw_walls(void) {
    /* Top + bottom walls */
    for (uint16_t x = BOARD_X; x < BOARD_X + BOARD_W; x++) {
        putCharAt(x, BOARD_Y,                  ' ', WALL_FG, WALL_BG);
        putCharAt(x, BOARD_Y + BOARD_H - 1,    ' ', WALL_FG, WALL_BG);
    }
    /* Left + right walls */
    for (uint16_t y = BOARD_Y; y < BOARD_Y + BOARD_H; y++) {
        putCharAt(BOARD_X,                  y, ' ', WALL_FG, WALL_BG);
        putCharAt(BOARD_X + BOARD_W - 1,    y, ' ', WALL_FG, WALL_BG);
    }
}

static void clear_board_interior(void) {
    for (uint16_t y = BOARD_Y + 1; y < BOARD_Y + BOARD_H - 1; y++) {
        for (uint16_t x = BOARD_X + 1; x < BOARD_X + BOARD_W - 1; x++) {
            putCharAt(x, y, ' ', VGA_COLOR_WHITE, PINK_BG);
        }
    }
}

static void draw_header(void) {
    draw_text_center(1, "================================================",
                     GOLD_TEXT, PINK_BG);
    draw_text_center(2, "  S N A K E   <3   M A R L E N  &  S A B I N A  ",
                     VGA_COLOR_WHITE, PINK_BG);
    draw_text_center(3, "================================================",
                     GOLD_TEXT, PINK_BG);

    /* Clear the score row first to avoid stale digits */
    for (uint16_t x = 0; x < 80; x++) {
        putCharAt(x, 4, ' ', VGA_COLOR_WHITE, PINK_BG);
    }

    char buf[16];
    uint_to_str(score, buf);
    draw_text(4, 4, "Score: ", VGA_COLOR_WHITE, PINK_BG);
    draw_text(11, 4, buf, GOLD_TEXT, PINK_BG);

    uint_to_str(high_score, buf);
    draw_text(60, 4, "Best: ", VGA_COLOR_WHITE, PINK_BG);
    draw_text(66, 4, buf, GOLD_TEXT, PINK_BG);
}

static void draw_footer(void) {
    draw_text_center(23, "Arrow keys to move  *  ESC to quit",
                     PINK_TEXT, PINK_BG);
    draw_text_center(24, "* ~ In Pink We Trust ~ *",
                     PINK_TEXT, PINK_BG);
}

static void draw_food(void) {
    /* Heart character (ASCII 3) */
    putCharAt(food.x, food.y, 3, FOOD_FG, FOOD_BG);
}

static void draw_snake_segment(int16_t x, int16_t y, int is_head) {
    if (is_head) {
        putCharAt(x, y, '@', SNAKE_HEAD_FG, SNAKE_HEAD_BG);
    } else {
        putCharAt(x, y, 'o', SNAKE_BODY_FG, SNAKE_BODY_BG);
    }
}

static void draw_full_snake(void) {
    for (uint16_t i = 0; i < snake_len; i++) {
        draw_snake_segment(snake[i].x, snake[i].y, i == 0);
    }
}

/* ================================================================== */
/* Food placement                                                      */
/* ================================================================== */

static int point_on_snake(int16_t x, int16_t y) {
    for (uint16_t i = 0; i < snake_len; i++) {
        if (snake[i].x == x && snake[i].y == y) return 1;
    }
    return 0;
}

static void place_food(void) {
    int16_t fx, fy;
    int tries = 0;
    do {
        fx = BOARD_X + 1 + (rnd() % (BOARD_W - 2));
        fy = BOARD_Y + 1 + (rnd() % (BOARD_H - 2));
        tries++;
        if (tries > 500) break;
    } while (point_on_snake(fx, fy));

    food.x = fx;
    food.y = fy;
    draw_food();
}

/* ================================================================== */
/* Game logic                                                          */
/* ================================================================== */

static void init_game(void) {
    snake_len = 4;
    int16_t cx = BOARD_X + BOARD_W / 2;
    int16_t cy = BOARD_Y + BOARD_H / 2;
    snake[0].x = cx;     snake[0].y = cy;
    snake[1].x = cx - 1; snake[1].y = cy;
    snake[2].x = cx - 2; snake[2].y = cy;
    snake[3].x = cx - 3; snake[3].y = cy;

    cur_dir = DIR_RIGHT;
    next_dir = DIR_RIGHT;
    score = 0;
    game_over = 0;
}

static void apply_input(uint8_t sc) {
    switch (sc) {
        case SC_UP:
            if (cur_dir != DIR_DOWN) next_dir = DIR_UP;
            break;
        case SC_DOWN:
            if (cur_dir != DIR_UP) next_dir = DIR_DOWN;
            break;
        case SC_LEFT:
            if (cur_dir != DIR_RIGHT) next_dir = DIR_LEFT;
            break;
        case SC_RIGHT:
            if (cur_dir != DIR_LEFT) next_dir = DIR_RIGHT;
            break;
    }
}

/* Move the snake one tick. Returns 0 on game over. */
static int step_game(void) {
    cur_dir = next_dir;

    int16_t nx = snake[0].x;
    int16_t ny = snake[0].y;
    switch (cur_dir) {
        case DIR_UP:    ny--; break;
        case DIR_DOWN:  ny++; break;
        case DIR_LEFT:  nx--; break;
        case DIR_RIGHT: nx++; break;
    }

    /* Wall collision */
    if (nx <= BOARD_X || nx >= BOARD_X + BOARD_W - 1 ||
        ny <= BOARD_Y || ny >= BOARD_Y + BOARD_H - 1) {
        return 0;
    }

    /* Self collision (skip tail unless eating) */
    int ate_food = (nx == food.x && ny == food.y);
    uint16_t check_limit = ate_food ? snake_len : snake_len - 1;
    for (uint16_t i = 0; i < check_limit; i++) {
        if (snake[i].x == nx && snake[i].y == ny) return 0;
    }

    if (ate_food) {
        if (snake_len < MAX_SNAKE) {
            for (int16_t i = (int16_t)snake_len; i > 0; i--) {
                snake[i] = snake[i - 1];
            }
            snake_len++;
        } else {
            for (int16_t i = (int16_t)snake_len - 1; i > 0; i--) {
                snake[i] = snake[i - 1];
            }
        }
        snake[0].x = nx;
        snake[0].y = ny;

        score += 10;

        draw_header();
        draw_full_snake();
        place_food();
    } else {
        /* Just move: erase tail, shift body, draw new head */
        point_t old_tail = snake[snake_len - 1];
        putCharAt(old_tail.x, old_tail.y, ' ', VGA_COLOR_WHITE, PINK_BG);

        for (int16_t i = (int16_t)snake_len - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0].x = nx;
        snake[0].y = ny;

        if (snake_len > 1) {
            draw_snake_segment(snake[1].x, snake[1].y, 0);
        }
        draw_snake_segment(snake[0].x, snake[0].y, 1);
    }

    return 1;
}

/* ================================================================== */
/* Game over screen                                                    */
/* ================================================================== */

static void draw_game_over_box(void) {
    uint16_t bx = 25;
    uint16_t by = 9;
    uint16_t bw = 30;
    uint16_t bh = 8;

    /* Fill box */
    for (uint16_t y = 0; y < bh; y++) {
        for (uint16_t x = 0; x < bw; x++) {
            putCharAt(bx + x, by + y, ' ',
                      VGA_COLOR_BLACK, VGA_COLOR_WHITE);
        }
    }
    /* Border */
    for (uint16_t x = 0; x < bw; x++) {
        putCharAt(bx + x, by,          ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
        putCharAt(bx + x, by + bh - 1, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
    }
    for (uint16_t y = 0; y < bh; y++) {
        putCharAt(bx,          y + by, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
        putCharAt(bx + bw - 1, y + by, ' ', VGA_COLOR_WHITE, VGA_COLOR_LIGHT_RED);
    }

    draw_text_center(by + 2, "G A M E   O V E R",
                     VGA_COLOR_LIGHT_RED, VGA_COLOR_WHITE);

    char buf[16];
    uint_to_str(score, buf);
    uint16_t score_len = 7 + slen(buf);
    uint16_t score_x = (80 - score_len) / 2;
    draw_text(score_x,     by + 4, "Score: ",
              VGA_COLOR_BLACK, VGA_COLOR_WHITE);
    draw_text(score_x + 7, by + 4, buf,
              VGA_COLOR_LIGHT_RED, VGA_COLOR_WHITE);

    draw_text_center(by + 6, "ENTER = retry  *  ESC = menu",
                     VGA_COLOR_BLACK, VGA_COLOR_WHITE);
}

/* ================================================================== */
/* Main game function                                                  */
/* ================================================================== */

void run_snake(void) {
    keyboard_set_echo(false);

    while (1) {
        rng_advance_seed();

        /* Set up new game */
        fillScreen(VGA_COLOR_WHITE, PINK_BG);
        draw_header();
        draw_walls();
        clear_board_interior();
        draw_footer();

        init_game();
        place_food();
        draw_full_snake();

        /* Game loop */
        while (!game_over) {
            uint32_t tick_ms = 120;
            if (snake_len > 10) tick_ms = 100;
            if (snake_len > 20) tick_ms = 85;
            if (snake_len > 35) tick_ms = 70;

            /* Poll input in small slices for responsive direction changes */
            uint32_t slice = 20;
            uint32_t elapsed = 0;
            while (elapsed < tick_ms) {
                uint8_t sc = keyboard_get_scancode();
                if (sc == SC_ESC) {
                    return;  /* exit to menu */
                }
                apply_input(sc);
                sleep_interrupt(slice);
                elapsed += slice;
            }

            if (!step_game()) {
                game_over = 1;
                break;
            }
        }

        /* Game over — update high score */
        if (score > high_score) high_score = score;

        draw_game_over_box();

        /* Wait for ENTER (retry) or ESC (back to menu) */
        while (1) {
            uint8_t sc = keyboard_get_scancode();
            if (sc == SC_ESC)   return;
            if (sc == SC_ENTER) break;
            sleep_interrupt(50);
        }
        /* loop back, start a new game */
    }
}