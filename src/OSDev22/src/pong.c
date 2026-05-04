/*
 * pong.c - Pink Pong Game
 *
 * Single-player vs AI opponent with PC speaker sound effects.
 *
 * Controls:
 *   W / S or UP / DOWN  - move your paddle
 *   1 / 2 / 3           - change difficulty (Easy / Medium / Hard)
 *   SPACE               - pause / resume
 *   R                   - restart
 *   ESC                 - return to menu
 *
 * Uses fixed-point integer math (1/8 cell precision) to avoid float.
 */

#include "pong.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "libc/stdint.h"

extern void sleep_interrupt(uint32_t ms);

/* ================================================================== */
/* Scancodes                                                           */
/* ================================================================== */

#define SC_W       0x11
#define SC_S       0x1F
#define SC_UP      0x48
#define SC_DOWN    0x50
#define SC_SPACE   0x39
#define SC_R       0x13
#define SC_1       0x02
#define SC_2       0x03
#define SC_3       0x04
#define SC_ESC     0x01
#define SC_ENTER   0x1C

/* ================================================================== */
/* Constants                                                           */
/* ================================================================== */

#define PINK_BG          VGA_COLOR_LIGHT_MAGENTA
#define PINK_TEXT        VGA_COLOR_LIGHT_BROWN
#define GOLD_TEXT        VGA_COLOR_BROWN

/* Court dimensions */
#define COURT_X          2
#define COURT_Y          4
#define COURT_W          76
#define COURT_H          18

/* Paddles */
#define PADDLE_H         4
#define LEFT_PADDLE_X    (COURT_X + 2)
#define RIGHT_PADDLE_X   (COURT_X + COURT_W - 3)

/* Win condition */
#define WIN_SCORE        7

/* Fixed-point: 1/8 cell precision */
#define FP_SHIFT         3
#define FP_ONE           (1 << FP_SHIFT)
#define TO_CELL(fp)      ((fp) >> FP_SHIFT)
#define FROM_CELL(c)     ((c) << FP_SHIFT)

/* Court interior bounds (in cells) */
#define COURT_TOP        (COURT_Y + 1)
#define COURT_BOTTOM     (COURT_Y + COURT_H - 2)
#define COURT_LEFT       (COURT_X + 1)
#define COURT_RIGHT      (COURT_X + COURT_W - 2)

/* Difficulty */
typedef enum {
    DIFF_EASY   = 0,
    DIFF_MEDIUM = 1,
    DIFF_HARD   = 2,
} difficulty_t;

/* AI tracking speed in fp per tick (higher = harder) */
static const int ai_speed[3] = { 2, 4, 6 };

/* AI tracking error (higher = sloppier) */
static const int ai_error[3] = { 4, 2, 0 };

/* Player paddle step (cells per keypress) */
#define PLAYER_STEP      2

/* Colors */
#define FRAME_FG         VGA_COLOR_BLACK
#define FRAME_BG         VGA_COLOR_LIGHT_GREY

#define PADDLE_L_FG      VGA_COLOR_WHITE
#define PADDLE_L_BG      VGA_COLOR_LIGHT_RED

#define PADDLE_R_FG      VGA_COLOR_WHITE
#define PADDLE_R_BG      VGA_COLOR_LIGHT_BLUE

#define BALL_FG          VGA_COLOR_BLACK
#define BALL_BG          VGA_COLOR_WHITE

#define NET_FG           VGA_COLOR_LIGHT_BROWN

/* ================================================================== */
/* PC Speaker (same approach as piano.c)                               */
/* ================================================================== */

#define PIT_BASE_FREQUENCY 1193180

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void spk_play(uint32_t freq) {
    if (freq == 0) return;
    uint32_t divisor = PIT_BASE_FREQUENCY / freq;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) outb(0x61, tmp | 3);
}

static void spk_stop(void) {
    uint8_t tmp = inb(0x61);
    outb(0x61, tmp & 0xFC);
}

/* Sound effects — short blips so they don't slow down gameplay */
static void sfx_wall_bounce(void) {
    /* Low click */
    spk_play(220);
    sleep_interrupt(15);
    spk_stop();
}

static void sfx_paddle_hit(void) {
    /* Mid-pitched chirp */
    spk_play(440);
    sleep_interrupt(20);
    spk_stop();
}

static void sfx_player_score(void) {
    /* Happy ascending chirp */
    spk_play(523);  /* C5 */
    sleep_interrupt(60);
    spk_play(659);  /* E5 */
    sleep_interrupt(60);
    spk_play(784);  /* G5 */
    sleep_interrupt(80);
    spk_stop();
}

static void sfx_ai_score(void) {
    /* Sad descending blip */
    spk_play(330);
    sleep_interrupt(80);
    spk_play(220);
    sleep_interrupt(120);
    spk_stop();
}

static void sfx_win(void) {
    /* Victory fanfare */
    spk_play(523);  sleep_interrupt(100);
    spk_play(659);  sleep_interrupt(100);
    spk_play(784);  sleep_interrupt(100);
    spk_play(1047); sleep_interrupt(200);
    spk_stop();
}

static void sfx_lose(void) {
    spk_play(392);  sleep_interrupt(150);
    spk_play(311);  sleep_interrupt(150);
    spk_play(247);  sleep_interrupt(250);
    spk_stop();
}

/* ================================================================== */
/* Game state                                                          */
/* ================================================================== */

typedef struct {
    int16_t x, y;       /* fixed-point */
    int16_t vx, vy;     /* fixed-point per tick */
} ball_t;

typedef struct {
    int16_t y;          /* top row, in cells */
    int16_t prev_y;
} paddle_t;

static ball_t   ball;
static int16_t  prev_ball_cx, prev_ball_cy;

static paddle_t left_paddle;   /* player */
static paddle_t right_paddle;  /* AI */

static uint8_t  player_score;
static uint8_t  ai_score;

static difficulty_t difficulty;
static uint8_t  paused;
static uint8_t  game_over;
static uint8_t  player_won;

/* Tiny RNG */
static uint32_t rng_state = 0xFEEDC0DE;
static uint32_t rnd(void) {
    rng_state = rng_state * 1103515245 + 12345;
    return (rng_state >> 16) & 0x7FFF;
}

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

static void uint_to_str(uint32_t n, char* buf) {
    if (n == 0) { buf[0] = '0'; buf[1] = 0; return; }
    char tmp[8];
    int i = 0;
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = 0;
}

/* ================================================================== */
/* Drawing                                                             */
/* ================================================================== */

static void draw_court_frame(void) {
    /* Top & bottom */
    for (uint16_t x = COURT_X; x < COURT_X + COURT_W; x++) {
        putCharAt(x, COURT_Y,                 ' ', FRAME_FG, FRAME_BG);
        putCharAt(x, COURT_Y + COURT_H - 1,   ' ', FRAME_FG, FRAME_BG);
    }
    /* Left & right */
    for (uint16_t y = COURT_Y; y < COURT_Y + COURT_H; y++) {
        putCharAt(COURT_X,                 y, ' ', FRAME_FG, FRAME_BG);
        putCharAt(COURT_X + COURT_W - 1,   y, ' ', FRAME_FG, FRAME_BG);
    }
}

static void clear_court_interior(void) {
    for (uint16_t y = COURT_TOP; y <= COURT_BOTTOM; y++) {
        for (uint16_t x = COURT_LEFT; x <= COURT_RIGHT; x++) {
            putCharAt(x, y, ' ', VGA_COLOR_WHITE, PINK_BG);
        }
    }
    /* Center net */
    int net_x = COURT_X + COURT_W / 2;
    for (uint16_t y = COURT_TOP; y <= COURT_BOTTOM; y++) {
        if ((y - COURT_TOP) % 2 == 0) {
            putCharAt(net_x, y, ':', NET_FG, PINK_BG);
        }
    }
}

static void redraw_net_at(int16_t x, int16_t y) {
    int net_x = COURT_X + COURT_W / 2;
    if (x == net_x && (y - COURT_TOP) % 2 == 0 &&
        y >= COURT_TOP && y <= COURT_BOTTOM) {
        putCharAt(x, y, ':', NET_FG, PINK_BG);
    } else {
        putCharAt(x, y, ' ', VGA_COLOR_WHITE, PINK_BG);
    }
}

static void draw_paddle(int16_t x, int16_t y, uint8_t fg, uint8_t bg) {
    for (int i = 0; i < PADDLE_H; i++) {
        putCharAt(x, y + i, ' ', fg, bg);
    }
}

static void erase_paddle(int16_t x, int16_t y) {
    for (int i = 0; i < PADDLE_H; i++) {
        putCharAt(x, y + i, ' ', VGA_COLOR_WHITE, PINK_BG);
    }
}

static void draw_ball(int16_t cx, int16_t cy) {
    putCharAt(cx, cy, 'O', BALL_FG, BALL_BG);
}

static void draw_header(void) {
    /* Title bar */
    draw_text_center(1, "================================================",
                     GOLD_TEXT, PINK_BG);
    draw_text_center(2, "          ~  P I N K   P O N G  ~          ",
                     VGA_COLOR_WHITE, PINK_BG);

    /* Scoreboard row */
    for (uint16_t x = 0; x < 80; x++) {
        putCharAt(x, 3, ' ', VGA_COLOR_WHITE, PINK_BG);
    }

    char buf[8];

    /* Player score (left) */
    draw_text(COURT_X + 2, 3, "YOU: ", VGA_COLOR_WHITE, PINK_BG);
    uint_to_str(player_score, buf);
    draw_text(COURT_X + 7, 3, buf, VGA_COLOR_LIGHT_RED, PINK_BG);

    /* AI score (right) */
    uint_to_str(ai_score, buf);
    uint16_t ai_label_x = COURT_X + COURT_W - 8;
    draw_text(ai_label_x,     3, "AI: ", VGA_COLOR_WHITE, PINK_BG);
    draw_text(ai_label_x + 4, 3, buf,    VGA_COLOR_LIGHT_BLUE, PINK_BG);

    /* Difficulty (center) */
    const char* diff_str = "MEDIUM";
    if (difficulty == DIFF_EASY)   diff_str = "EASY  ";
    if (difficulty == DIFF_HARD)   diff_str = "HARD  ";
    draw_text(36, 3, "[ ", PINK_TEXT, PINK_BG);
    draw_text(38, 3, diff_str, GOLD_TEXT, PINK_BG);
    draw_text(44, 3, " ]", PINK_TEXT, PINK_BG);
}

static void draw_footer(void) {
    for (uint16_t x = 0; x < 80; x++) {
        putCharAt(x, 23, ' ', VGA_COLOR_WHITE, PINK_BG);
        putCharAt(x, 24, ' ', VGA_COLOR_WHITE, PINK_BG);
    }
    draw_text_center(23,
        "W/S or UP/DOWN  *  1/2/3 difficulty  *  SPACE pause  *  R reset",
        VGA_COLOR_WHITE, PINK_BG);
    draw_text_center(24, "* ~ ESC to return ~ *", PINK_TEXT, PINK_BG);
}

/* ================================================================== */
/* Game logic                                                          */
/* ================================================================== */

static void serve_ball(int to_right) {
    ball.x = FROM_CELL(COURT_X + COURT_W / 2);
    ball.y = FROM_CELL(COURT_Y + COURT_H / 2);

    /* Velocity: ~1.0 cells/tick horizontal, randomized vertical slope */
    ball.vx = to_right ? FP_ONE : -FP_ONE;
    int16_t vy_choices[5] = { -FP_ONE / 2, -FP_ONE / 4, 0,
                               FP_ONE / 4,  FP_ONE / 2 };
    ball.vy = vy_choices[rnd() % 5];

    prev_ball_cx = TO_CELL(ball.x);
    prev_ball_cy = TO_CELL(ball.y);
}

static void reset_match(void) {
    player_score = 0;
    ai_score = 0;
    game_over = 0;
    player_won = 0;
    paused = 0;

    left_paddle.y      = COURT_Y + COURT_H / 2 - PADDLE_H / 2;
    left_paddle.prev_y = left_paddle.y;
    right_paddle.y      = left_paddle.y;
    right_paddle.prev_y = left_paddle.y;

    serve_ball(rnd() & 1);
}

static void clamp_paddle(paddle_t* p) {
    if (p->y < COURT_TOP) p->y = COURT_TOP;
    if (p->y + PADDLE_H - 1 > COURT_BOTTOM)
        p->y = COURT_BOTTOM - PADDLE_H + 1;
}

static void update_paddle_visual(paddle_t* p, int16_t x,
                                  uint8_t fg, uint8_t bg) {
    if (p->y == p->prev_y) return;
    erase_paddle(x, p->prev_y);
    draw_paddle(x, p->y, fg, bg);
    p->prev_y = p->y;
}

/* AI tracking — moves toward the ball with some imprecision */
static void update_ai(void) {
    int16_t ball_cy = TO_CELL(ball.y);
    int16_t paddle_center = right_paddle.y + PADDLE_H / 2;

    /* Only react when ball comes toward AI */
    int16_t target;
    if (ball.vx > 0) {
        target = ball_cy;
        /* Add error: small random offset based on difficulty */
        int err = ai_error[difficulty];
        if (err > 0) {
            target += (int16_t)(rnd() % (2 * err + 1)) - err;
        }
    } else {
        /* Drift back toward center */
        target = COURT_Y + COURT_H / 2;
    }

    int16_t speed = ai_speed[difficulty];
    /* Convert paddle move (in fp) to cells, but limit to integer per tick */
    int16_t move_cells = (speed + FP_ONE / 2) / FP_ONE;
    if (move_cells < 1) move_cells = 1;

    if (paddle_center < target - 1) {
        right_paddle.y += move_cells;
    } else if (paddle_center > target + 1) {
        right_paddle.y -= move_cells;
    }

    clamp_paddle(&right_paddle);
}

/* Returns 1 if a score happened this tick (game state changed a lot) */
static int update_ball(void) {
    /* Erase old ball position */
    redraw_net_at(prev_ball_cx, prev_ball_cy);

    /* Move */
    ball.x += ball.vx;
    ball.y += ball.vy;

    int16_t cx = TO_CELL(ball.x);
    int16_t cy = TO_CELL(ball.y);

    /* Top/bottom wall bounce */
    if (cy <= COURT_TOP) {
        ball.y = FROM_CELL(COURT_TOP + 1);
        ball.vy = -ball.vy;
        cy = TO_CELL(ball.y);
        sfx_wall_bounce();
    } else if (cy >= COURT_BOTTOM) {
        ball.y = FROM_CELL(COURT_BOTTOM - 1);
        ball.vy = -ball.vy;
        cy = TO_CELL(ball.y);
        sfx_wall_bounce();
    }

    /* Left paddle collision */
    if (cx <= LEFT_PADDLE_X + 1 && ball.vx < 0) {
        if (cy >= left_paddle.y && cy < left_paddle.y + PADDLE_H) {
            ball.vx = -ball.vx;
            /* Add english based on where it hit the paddle */
            int16_t hit_offset = cy - (left_paddle.y + PADDLE_H / 2);
            ball.vy += hit_offset * (FP_ONE / 4);
            /* Speed up slightly, capped */
            if (ball.vx < FP_ONE * 2) ball.vx += FP_ONE / 8;
            sfx_paddle_hit();
            cx = TO_CELL(ball.x);
        }
    }

    /* Right paddle collision */
    if (cx >= RIGHT_PADDLE_X - 1 && ball.vx > 0) {
        if (cy >= right_paddle.y && cy < right_paddle.y + PADDLE_H) {
            ball.vx = -ball.vx;
            int16_t hit_offset = cy - (right_paddle.y + PADDLE_H / 2);
            ball.vy += hit_offset * (FP_ONE / 4);
            if (ball.vx > -FP_ONE * 2) ball.vx -= FP_ONE / 8;
            sfx_paddle_hit();
            cx = TO_CELL(ball.x);
        }
    }

    /* Cap vertical velocity */
    if (ball.vy >  FP_ONE) ball.vy =  FP_ONE;
    if (ball.vy < -FP_ONE) ball.vy = -FP_ONE;

    /* Score: ball past paddle */
    if (cx < COURT_LEFT) {
        ai_score++;
        sfx_ai_score();
        if (ai_score >= WIN_SCORE) {
            game_over = 1;
            player_won = 0;
        } else {
            serve_ball(0);  /* serve toward player who lost */
        }
        return 1;
    } else if (cx > COURT_RIGHT) {
        player_score++;
        sfx_player_score();
        if (player_score >= WIN_SCORE) {
            game_over = 1;
            player_won = 1;
        } else {
            serve_ball(1);
        }
        return 1;
    }

    /* Draw new ball */
    draw_ball(cx, cy);
    prev_ball_cx = cx;
    prev_ball_cy = cy;
    return 0;
}

/* ================================================================== */
/* Input                                                               */
/* ================================================================== */

static int handle_input(void) {
    /* Returns: 0 = continue, 1 = exit to menu, 2 = full restart */
    uint8_t sc = keyboard_get_scancode();
    if (sc == 0) return 0;

    switch (sc) {
        case SC_ESC:
            spk_stop();
            return 1;

        case SC_W:
        case SC_UP:
            left_paddle.y -= PLAYER_STEP;
            clamp_paddle(&left_paddle);
            break;

        case SC_S:
        case SC_DOWN:
            left_paddle.y += PLAYER_STEP;
            clamp_paddle(&left_paddle);
            break;

        case SC_SPACE:
            paused = !paused;
            break;

        case SC_R:
            return 2;

        case SC_1:
            difficulty = DIFF_EASY;
            draw_header();
            break;
        case SC_2:
            difficulty = DIFF_MEDIUM;
            draw_header();
            break;
        case SC_3:
            difficulty = DIFF_HARD;
            draw_header();
            break;
    }

    return 0;
}

/* ================================================================== */
/* Game over screen                                                    */
/* ================================================================== */

static void draw_game_over(void) {
    uint16_t bx = 25;
    uint16_t by = 9;
    uint16_t bw = 30;
    uint16_t bh = 8;

    uint8_t border_color = player_won
        ? VGA_COLOR_LIGHT_RED   /* pink-ish */
        : VGA_COLOR_LIGHT_BLUE; /* AI-blue */

    /* Fill */
    for (uint16_t y = 0; y < bh; y++) {
        for (uint16_t x = 0; x < bw; x++) {
            putCharAt(bx + x, by + y, ' ',
                      VGA_COLOR_BLACK, VGA_COLOR_WHITE);
        }
    }
    /* Border */
    for (uint16_t x = 0; x < bw; x++) {
        putCharAt(bx + x, by,          ' ', VGA_COLOR_WHITE, border_color);
        putCharAt(bx + x, by + bh - 1, ' ', VGA_COLOR_WHITE, border_color);
    }
    for (uint16_t y = 0; y < bh; y++) {
        putCharAt(bx,          by + y, ' ', VGA_COLOR_WHITE, border_color);
        putCharAt(bx + bw - 1, by + y, ' ', VGA_COLOR_WHITE, border_color);
    }

    if (player_won) {
        draw_text_center(by + 2, "Y O U   W I N !",
                         VGA_COLOR_LIGHT_RED, VGA_COLOR_WHITE);
    } else {
        draw_text_center(by + 2, "A I   W I N S",
                         VGA_COLOR_LIGHT_BLUE, VGA_COLOR_WHITE);
    }

    char pbuf[8], abuf[8];
    uint_to_str(player_score, pbuf);
    uint_to_str(ai_score, abuf);
    /* "YOU N - N AI" centered */
    char line[32];
    int p = 0;
    line[p++] = 'Y'; line[p++] = 'O'; line[p++] = 'U'; line[p++] = ' ';
    for (int i = 0; pbuf[i]; i++) line[p++] = pbuf[i];
    line[p++] = ' '; line[p++] = '-'; line[p++] = ' ';
    for (int i = 0; abuf[i]; i++) line[p++] = abuf[i];
    line[p++] = ' '; line[p++] = 'A'; line[p++] = 'I';
    line[p] = 0;

    draw_text_center(by + 4, line, VGA_COLOR_BLACK, VGA_COLOR_WHITE);

    draw_text_center(by + 6, "ENTER = play again  *  ESC = menu",
                     VGA_COLOR_BLACK, VGA_COLOR_WHITE);
}

/* ================================================================== */
/* Main entry                                                          */
/* ================================================================== */

void run_pong(void) {
    keyboard_set_echo(false);
    spk_stop();

    difficulty = DIFF_MEDIUM;

    while (1) {
        /* Setup new match */
        fillScreen(VGA_COLOR_WHITE, PINK_BG);
        draw_header();
        draw_court_frame();
        clear_court_interior();
        draw_footer();

        reset_match();

        /* Initial paddle draw */
        draw_paddle(LEFT_PADDLE_X,  left_paddle.y,
                    PADDLE_L_FG, PADDLE_L_BG);
        draw_paddle(RIGHT_PADDLE_X, right_paddle.y,
                    PADDLE_R_FG, PADDLE_R_BG);
        draw_ball(TO_CELL(ball.x), TO_CELL(ball.y));

        /* Game loop */
        while (!game_over) {
            int input_action = handle_input();
            if (input_action == 1) { spk_stop(); return; }      /* ESC */
            if (input_action == 2) break;                        /* R - restart */

            if (!paused) {
                update_ai();

                /* Update paddle visuals */
                update_paddle_visual(&left_paddle, LEFT_PADDLE_X,
                                     PADDLE_L_FG, PADDLE_L_BG);
                update_paddle_visual(&right_paddle, RIGHT_PADDLE_X,
                                     PADDLE_R_FG, PADDLE_R_BG);

                /* Update ball — may score and reserve */
                if (update_ball()) {
                    /* Score happened — redraw header for new score */
                    draw_header();
                    /* Repaint paddles in case they got overlapped */
                    draw_paddle(LEFT_PADDLE_X,  left_paddle.y,
                                PADDLE_L_FG, PADDLE_L_BG);
                    draw_paddle(RIGHT_PADDLE_X, right_paddle.y,
                                PADDLE_R_FG, PADDLE_R_BG);
                    /* Brief pause after a point */
                    if (!game_over) sleep_interrupt(400);
                }
            } else {
                /* Pause indicator */
                draw_text_center(COURT_Y + COURT_H / 2,
                                 "  P A U S E D  ",
                                 VGA_COLOR_BLACK, VGA_COLOR_WHITE);
            }

            sleep_interrupt(40);
        }

        /* If we broke out via R-restart, loop back */
        if (!game_over) continue;

        /* Real game over — fanfare and game over box */
        if (player_won) sfx_win();
        else            sfx_lose();

        draw_game_over();

        /* Wait for ENTER (replay) or ESC (menu) */
        while (1) {
            uint8_t sc = keyboard_get_scancode();
            if (sc == SC_ESC)   { spk_stop(); return; }
            if (sc == SC_ENTER) break;
            sleep_interrupt(50);
        }
        /* Fall through to top of while(1) → new match */
    }
}