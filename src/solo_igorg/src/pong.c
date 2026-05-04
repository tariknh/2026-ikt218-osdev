#include <pong.h>
#include <terminal.h>
#include <keyboard.h>
#include <memory.h>
#include <pit.h>
#include <io.h>
#include <libc/stdint.h>
#include <libc/stddef.h>
#include <libc/stdbool.h>

#define PONG_FIELD_LEFT 2
#define PONG_FIELD_TOP 2
#define PONG_FIELD_WIDTH 60
#define PONG_FIELD_HEIGHT 20

#define PONG_PADDLE_HEIGHT 4
#define PONG_LEFT_PADDLE_X (PONG_FIELD_LEFT + 2)
#define PONG_RIGHT_PADDLE_X (PONG_FIELD_LEFT + PONG_FIELD_WIDTH - 3)

#define PONG_BALL_START_X (PONG_FIELD_LEFT + PONG_FIELD_WIDTH / 2)
#define PONG_BALL_START_Y (PONG_FIELD_TOP + PONG_FIELD_HEIGHT / 2)

#define PONG_WIN_SCORE 5

#define COLOR_DEFAULT 0x0F
#define COLOR_BORDER  0x0B
#define COLOR_PADDLE  0x0A
#define COLOR_BALL    0x0E
#define COLOR_TEXT    0x0F

#define PC_SPEAKER_PORT 0x61
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL2_PORT 0x42
#define PIT_BASE_FREQUENCY 1193182

typedef struct {
    int ball_x;
    int ball_y;
    int ball_dx;
    int ball_dy;

    int left_paddle_y;
    int right_paddle_y;

    uint32_t left_score;
    uint32_t right_score;

    bool paused;
    bool running;
} PongState;

static PongState* game = 0;

static void pong_enable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);

    if ((state & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, state | 0x03);
    }
}

static void pong_disable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & 0xFC);
}

static void pong_beep(uint32_t frequency, uint32_t duration_ms)
{
    if (frequency == 0) {
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_COMMAND_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    pong_enable_speaker();
    sleep_interrupt(duration_ms);
    pong_disable_speaker();
}

static void pong_reset_ball(int direction)
{
    game->ball_x = PONG_BALL_START_X;
    game->ball_y = PONG_BALL_START_Y;
    game->ball_dx = direction;
    game->ball_dy = 1;
}

static void pong_reset_game(void)
{
    game->left_score = 0;
    game->right_score = 0;
    game->left_paddle_y = PONG_FIELD_TOP + (PONG_FIELD_HEIGHT / 2) - (PONG_PADDLE_HEIGHT / 2);
    game->right_paddle_y = game->left_paddle_y;
    game->paused = false;
    game->running = true;

    pong_reset_ball(1);
}

static void pong_draw_number(uint32_t number, size_t x, size_t y)
{
    if (number >= 10) {
        terminal_putchar_at('0' + (number / 10), COLOR_TEXT, x, y);
        terminal_putchar_at('0' + (number % 10), COLOR_TEXT, x + 1, y);
    } else {
        terminal_putchar_at('0' + number, COLOR_TEXT, x, y);
    }
}

static void pong_draw_text(const char* text, size_t x, size_t y)
{
    for (size_t i = 0; text[i] != '\0'; i++) {
        terminal_putchar_at(text[i], COLOR_TEXT, x + i, y);
    }
}

static void pong_draw_border(void)
{
    for (size_t x = PONG_FIELD_LEFT; x < PONG_FIELD_LEFT + PONG_FIELD_WIDTH; x++) {
        terminal_putchar_at('-', COLOR_BORDER, x, PONG_FIELD_TOP);
        terminal_putchar_at('-', COLOR_BORDER, x, PONG_FIELD_TOP + PONG_FIELD_HEIGHT - 1);
    }

    for (size_t y = PONG_FIELD_TOP; y < PONG_FIELD_TOP + PONG_FIELD_HEIGHT; y++) {
        terminal_putchar_at('|', COLOR_BORDER, PONG_FIELD_LEFT, y);
        terminal_putchar_at('|', COLOR_BORDER, PONG_FIELD_LEFT + PONG_FIELD_WIDTH - 1, y);
    }
}

static void pong_draw_paddle(int x, int y)
{
    for (int i = 0; i < PONG_PADDLE_HEIGHT; i++) {
        terminal_putchar_at('|', COLOR_PADDLE, (size_t)x, (size_t)(y + i));
    }
}

static void pong_draw(void)
{
    terminal_clear();

    pong_draw_text("OS Pong - W/S move, P pause, R reset", 2, 0);

    pong_draw_border();

    pong_draw_text("Player:", 8, 1);
    pong_draw_number(game->left_score, 16, 1);

    pong_draw_text("AI:", 42, 1);
    pong_draw_number(game->right_score, 46, 1);

    pong_draw_paddle(PONG_LEFT_PADDLE_X, game->left_paddle_y);
    pong_draw_paddle(PONG_RIGHT_PADDLE_X, game->right_paddle_y);

    terminal_putchar_at('O', COLOR_BALL, (size_t)game->ball_x, (size_t)game->ball_y);

    if (game->paused) {
        pong_draw_text("PAUSED", PONG_FIELD_LEFT + 26, PONG_FIELD_TOP + 10);
    }

    if (game->left_score >= PONG_WIN_SCORE) {
        pong_draw_text("PLAYER WINS - PRESS R", PONG_FIELD_LEFT + 19, PONG_FIELD_TOP + 10);
    }

    if (game->right_score >= PONG_WIN_SCORE) {
        pong_draw_text("AI WINS - PRESS R", PONG_FIELD_LEFT + 21, PONG_FIELD_TOP + 10);
    }
}

static void pong_handle_input(void)
{
    char key = keyboard_get_last_key();

    if (key == 0) {
        return;
    }

    if (key == 'w') {
        if (game->left_paddle_y > PONG_FIELD_TOP + 1) {
            game->left_paddle_y--;
        }
    } else if (key == 's') {
        if (game->left_paddle_y + PONG_PADDLE_HEIGHT < PONG_FIELD_TOP + PONG_FIELD_HEIGHT - 1) {
            game->left_paddle_y++;
        }
    } else if (key == 'p') {
        game->paused = !game->paused;
    } else if (key == 'r') {
        pong_reset_game();
    }
}

static void pong_update_ai(void)
{
    int paddle_center = game->right_paddle_y + (PONG_PADDLE_HEIGHT / 2);

    if (game->ball_y < paddle_center &&
        game->right_paddle_y > PONG_FIELD_TOP + 1) {
        game->right_paddle_y--;
    } else if (game->ball_y > paddle_center &&
               game->right_paddle_y + PONG_PADDLE_HEIGHT < PONG_FIELD_TOP + PONG_FIELD_HEIGHT - 1) {
        game->right_paddle_y++;
    }
}

static bool pong_ball_hits_paddle(int paddle_x, int paddle_y)
{
    if (game->ball_x != paddle_x) {
        return false;
    }

    return game->ball_y >= paddle_y &&
           game->ball_y < paddle_y + PONG_PADDLE_HEIGHT;
}

static void pong_update_ball(void)
{
    if (game->paused ||
        game->left_score >= PONG_WIN_SCORE ||
        game->right_score >= PONG_WIN_SCORE) {
        return;
    }

    game->ball_x += game->ball_dx;
    game->ball_y += game->ball_dy;

    if (game->ball_y <= PONG_FIELD_TOP + 1 ||
        game->ball_y >= PONG_FIELD_TOP + PONG_FIELD_HEIGHT - 2) {
        game->ball_dy = -game->ball_dy;
        pong_beep(600, 20);
    }

    if (pong_ball_hits_paddle(PONG_LEFT_PADDLE_X + 1, game->left_paddle_y)) {
        game->ball_dx = 1;
        pong_beep(900, 20);
    }

    if (pong_ball_hits_paddle(PONG_RIGHT_PADDLE_X - 1, game->right_paddle_y)) {
        game->ball_dx = -1;
        pong_beep(900, 20);
    }

    if (game->ball_x <= PONG_FIELD_LEFT + 1) {
        game->right_score++;
        pong_beep(250, 100);
        pong_reset_ball(1);
    }

    if (game->ball_x >= PONG_FIELD_LEFT + PONG_FIELD_WIDTH - 2) {
        game->left_score++;
        pong_beep(250, 100);
        pong_reset_ball(-1);
    }
}

void pong_start(void)
{
    keyboard_set_echo(false);

    game = (PongState*)malloc(sizeof(PongState));

    if (game == 0) {
        terminal_write("Could not allocate Pong state\n");
        return;
    }

    pong_reset_game();
    pong_draw();

    while (1) {
        pong_handle_input();
        pong_update_ai();
        pong_update_ball();
        pong_draw();

        sleep_interrupt(80);
    }
}
