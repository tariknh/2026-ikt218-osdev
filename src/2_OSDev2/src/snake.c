#include <libc/stddef.h>
#include <libc/stdint.h>

#include "snake.h"
#include "pit.h"
#include "terminal.h"

#define SNAKE_BOARD_WIDTH 30u
#define SNAKE_BOARD_HEIGHT 15u
#define SNAKE_MAX_SEGMENTS (SNAKE_BOARD_WIDTH * SNAKE_BOARD_HEIGHT)
#define SNAKE_MOVE_INTERVAL_MS 140u
#define SNAKE_GAME_OVER_SHOW_MS 2000u

typedef struct {
    size_t x;
    size_t y;
} SnakePoint;

typedef enum {
    SNAKE_STATE_IDLE = 0,
    SNAKE_STATE_PAUSED,
    SNAKE_STATE_RUNNING,
    SNAKE_STATE_GAME_OVER
} SnakeState;

static volatile SnakeState snake_state = SNAKE_STATE_IDLE;
static volatile int snake_exit_requested = 0;
static volatile int snake_needs_redraw = 0;

static uint32_t snake_rng_state = 1;
static uint32_t snake_start_ticks = 0;
static uint32_t snake_last_move_ticks = 0;
static uint32_t snake_apples_eaten = 0;

static SnakePoint snake_body[SNAKE_MAX_SEGMENTS];
static size_t snake_length = 0;
static size_t apple_x = 0;
static size_t apple_y = 0;
static int apple_present = 0;
static char snake_direction = 0;

static void terminal_write_char(char c) {
    char text[2] = { c, '\0' };
    terminal_write(text);
}

static uint32_t snake_rand(void) {
    snake_rng_state ^= snake_rng_state << 13;
    snake_rng_state ^= snake_rng_state >> 17;
    snake_rng_state ^= snake_rng_state << 5;
    return snake_rng_state;
}

static int snake_is_on_body(size_t x, size_t y) {
    for (size_t i = 0; i < snake_length; i++) {
        if (snake_body[i].x == x && snake_body[i].y == y) {
            return 1;
        }
    }

    return 0;
}

static int snake_is_opposite_direction(char a, char b) {
    if (b == 0) {
        return 0;
    }

    return (a == SNAKE_INPUT_UP && b == SNAKE_INPUT_DOWN) ||
           (a == SNAKE_INPUT_DOWN && b == SNAKE_INPUT_UP) ||
           (a == SNAKE_INPUT_LEFT && b == SNAKE_INPUT_RIGHT) ||
           (a == SNAKE_INPUT_RIGHT && b == SNAKE_INPUT_LEFT);
}

static void snake_spawn_apple(void) {
    size_t free_cells = (SNAKE_BOARD_WIDTH - 2u) * (SNAKE_BOARD_HEIGHT - 2u);

    if (snake_length >= free_cells) {
        apple_present = 0;
        snake_state = SNAKE_STATE_GAME_OVER;
        return;
    }

    size_t start = (size_t)(snake_rand() % free_cells);

    for (size_t i = 0; i < free_cells; i++) {
        size_t index = (start + i) % free_cells;
        size_t candidate_x = 1u + (index % (SNAKE_BOARD_WIDTH - 2u));
        size_t candidate_y = 1u + (index / (SNAKE_BOARD_WIDTH - 2u));

        if (!snake_is_on_body(candidate_x, candidate_y)) {
            apple_x = candidate_x;
            apple_y = candidate_y;
            apple_present = 1;
            return;
        }
    }

    apple_present = 0;
    snake_state = SNAKE_STATE_GAME_OVER;
}

static void snake_draw_status(void) {
    terminal_write("Snake\n");
    terminal_write("Apples eaten: ");
    terminal_write_dec(snake_apples_eaten);
    terminal_write("  Time played: ");
    terminal_write_dec((pit_get_ticks() - snake_start_ticks) / 1000u);
    terminal_write("s");

    if (snake_state == SNAKE_STATE_PAUSED) {
        terminal_write("  [Paused]");
    }

    terminal_write("\n");
    terminal_write("Movement key to start. Esc pauses, Esc again exits.\n\n");
}

static void snake_draw_board(void) {
    for (size_t y = 0; y < SNAKE_BOARD_HEIGHT; y++) {
        for (size_t x = 0; x < SNAKE_BOARD_WIDTH; x++) {
            char cell = ' ';

            if (x == 0 || y == 0 || x == SNAKE_BOARD_WIDTH - 1u || y == SNAKE_BOARD_HEIGHT - 1u) {
                cell = '#';
            } else if (apple_present && x == apple_x && y == apple_y) {
                cell = '@';
            } else {
                for (size_t i = 0; i < snake_length; i++) {
                    if (snake_body[i].x == x && snake_body[i].y == y) {
                        cell = (i == 0) ? 'O' : 'o';
                        break;
                    }
                }
            }

            terminal_write_char(cell);
        }

        terminal_write("\n");
    }
}

static void snake_redraw(void) {
    terminal_clear();
    snake_draw_status();
    snake_draw_board();

    if (snake_state == SNAKE_STATE_PAUSED) {
        terminal_write("\nPaused. Press Esc again to quit or a movement key to start.\n");
    }
}

static void snake_end_screen(void) {
    terminal_clear();
    terminal_write("Game over! Apples eaten: ");
    terminal_write_dec(snake_apples_eaten);
    terminal_write("; Time played: ");
    terminal_write_dec((pit_get_ticks() - snake_start_ticks) / 1000u);
    terminal_write("s\n");
}

static void snake_step(void) {
    if (snake_direction == 0) {
        return;
    }

    int16_t next_x = (int16_t)snake_body[0].x;
    int16_t next_y = (int16_t)snake_body[0].y;

    switch (snake_direction) {
        case SNAKE_INPUT_UP:
            next_y--;
            break;
        case SNAKE_INPUT_DOWN:
            next_y++;
            break;
        case SNAKE_INPUT_LEFT:
            next_x--;
            break;
        case SNAKE_INPUT_RIGHT:
            next_x++;
            break;
        default:
            return;
    }

    if (next_x <= 0 || next_y <= 0 || next_x >= (int16_t)(SNAKE_BOARD_WIDTH - 1u) || next_y >= (int16_t)(SNAKE_BOARD_HEIGHT - 1u)) {
        snake_state = SNAKE_STATE_GAME_OVER;
        return;
    }

    int grow = apple_present && (size_t)next_x == apple_x && (size_t)next_y == apple_y;
    size_t collision_limit = grow ? snake_length : (snake_length > 0 ? snake_length - 1u : 0u);

    for (size_t i = 0; i < collision_limit; i++) {
        if ((int16_t)snake_body[i].x == next_x && (int16_t)snake_body[i].y == next_y) {
            snake_state = SNAKE_STATE_GAME_OVER;
            return;
        }
    }

    if (grow) {
        if (snake_length >= SNAKE_MAX_SEGMENTS) {
            snake_state = SNAKE_STATE_GAME_OVER;
            return;
        }

        snake_apples_eaten++;
        snake_length++;
    }

    for (size_t i = snake_length - 1u; i > 0u; i--) {
        snake_body[i] = snake_body[i - 1u];
    }

    snake_body[0].x = (size_t)next_x;
    snake_body[0].y = (size_t)next_y;

    if (grow) {
        snake_spawn_apple();
    }

    snake_needs_redraw = 1;
}

int snake_is_running(void) {
    return snake_state != SNAKE_STATE_IDLE;
}

void snake_handle_input(char input) {
    if (snake_state == SNAKE_STATE_IDLE || snake_state == SNAKE_STATE_GAME_OVER) {
        return;
    }

    if (input == (char)SNAKE_INPUT_ESCAPE) {
        if (snake_state == SNAKE_STATE_PAUSED) {
            snake_exit_requested = 1;
        } else {
            snake_state = SNAKE_STATE_PAUSED;
            snake_needs_redraw = 1;
        }

        return;
    }

    if (input == 'w' || input == 'W') {
        input = (char)SNAKE_INPUT_UP;
    } else if (input == 's' || input == 'S') {
        input = (char)SNAKE_INPUT_DOWN;
    } else if (input == 'a' || input == 'A') {
        input = (char)SNAKE_INPUT_LEFT;
    } else if (input == 'd' || input == 'D') {
        input = (char)SNAKE_INPUT_RIGHT;
    }

    if (input != SNAKE_INPUT_UP && input != SNAKE_INPUT_DOWN && input != SNAKE_INPUT_LEFT && input != SNAKE_INPUT_RIGHT) {
        return;
    }

    if (snake_state == SNAKE_STATE_PAUSED) {
        snake_direction = input;
        snake_state = SNAKE_STATE_RUNNING;
        snake_last_move_ticks = pit_get_ticks();
        snake_needs_redraw = 1;
        return;
    }

    if (!snake_is_opposite_direction(input, snake_direction)) {
        snake_direction = input;
    }
}

void snake_run(void) {
    snake_state = SNAKE_STATE_PAUSED;
    snake_exit_requested = 0;
    snake_needs_redraw = 1;
    snake_apples_eaten = 0;
    snake_start_ticks = pit_get_ticks();
    snake_last_move_ticks = snake_start_ticks;
    snake_rng_state = snake_start_ticks ^ 0xA5A5A5A5u;
    snake_direction = 0;

    snake_length = 3u;
    snake_body[0].x = SNAKE_BOARD_WIDTH / 2u;
    snake_body[0].y = SNAKE_BOARD_HEIGHT / 2u;
    snake_body[1].x = snake_body[0].x - 1u;
    snake_body[1].y = snake_body[0].y;
    snake_body[2].x = snake_body[0].x - 2u;
    snake_body[2].y = snake_body[0].y;

    snake_spawn_apple();
    snake_redraw();

    while (!snake_exit_requested && snake_state != SNAKE_STATE_GAME_OVER) {
        uint32_t now = pit_get_ticks();

        if (snake_needs_redraw) {
            snake_needs_redraw = 0;
            snake_redraw();
        }

        if (snake_state == SNAKE_STATE_RUNNING && (uint32_t)(now - snake_last_move_ticks) >= SNAKE_MOVE_INTERVAL_MS) {
            snake_last_move_ticks = now;
            snake_step();
            continue;
        }

        __asm__ volatile ("sti; hlt");
    }

    if (snake_state == SNAKE_STATE_GAME_OVER) {
        snake_end_screen();
        sleep_interrupt(SNAKE_GAME_OVER_SHOW_MS);
    }

    terminal_clear();
    snake_state = SNAKE_STATE_IDLE;
}
