#include "snake.h"
#include "irq.h"
#include "pit.h"
#include "pcspk.h"
#include "terminal.h"
#include <libc/stdint.h>
#include <libc/stdbool.h>

// Board position and size
#define BOARD_LEFT   10
#define BOARD_TOP    3
#define BOARD_WIDTH  60
#define BOARD_HEIGHT 20

// Max snake size inside the board
#define SNAKE_MAX_CELLS ((BOARD_WIDTH - 2) * (BOARD_HEIGHT - 2))

// One cell on the board
typedef struct {
    int x;
    int y;
} point_t;

// Snake body and game state
static point_t snake_cells[SNAKE_MAX_CELLS];
static uint32_t snake_len;
static point_t food;
static int dir_x;
static int dir_y;
static uint32_t score;
// Small random seed for food placement
static uint32_t rng_state = 0xC0FFEEu;

// Short background melody for the game
static const struct note snake_notes[] = {
    { 523, 120 }, { 659, 120 }, { 784, 120 }, { 659, 120 },
    { 587, 120 }, { 698, 120 }, { 880, 120 }, { 698, 120 },
    { 523, 100 }, { 659, 100 }, { 784, 100 }, { 988, 180 },
    { 880, 120 }, { 784, 120 }, { 659, 120 }, { 523, 180 }
};

static const struct song snake_song = {
    .notes = snake_notes,
    .note_count = (uint32_t)(sizeof(snake_notes) / sizeof(snake_notes[0])),
    .name = "Snake Loop"
};

// Simple random number generator
static uint32_t rand_next(void) {
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

// Draw text at one screen position
static void draw_text(int x, int y, const char *text) {
    while (*text) {
        terminal_put_at(x++, y, *text++);
    }
}

// Convert a number to text
static void u32_to_dec(uint32_t v, char *out) {
    char tmp[16];
    int i = 0;

    if (v == 0u) {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while (v > 0u) {
        tmp[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }

    for (int j = 0; j < i; j++) {
        out[j] = tmp[i - j - 1];
    }
    out[i] = '\0';
}

// Check if two points are equal
static bool point_eq(point_t a, point_t b) {
    return a.x == b.x && a.y == b.y;
}

// Check if the snake already uses this cell
static bool snake_contains(point_t p) {
    for (uint32_t i = 0; i < snake_len; i++) {
        if (point_eq(snake_cells[i], p)) {
            return true;
        }
    }
    return false;
}

// Put new food on a free cell
static void spawn_food(void) {
    point_t candidate;

    do {
        candidate.x = BOARD_LEFT + 1 + (int)(rand_next() % (uint32_t)(BOARD_WIDTH - 2));
        candidate.y = BOARD_TOP + 1 + (int)(rand_next() % (uint32_t)(BOARD_HEIGHT - 2));
    } while (snake_contains(candidate));

    food = candidate;
}

// Draw the box around the board
static void draw_board(void) {
    for (int x = BOARD_LEFT; x < BOARD_LEFT + BOARD_WIDTH; x++) {
        terminal_put_at(x, BOARD_TOP, '#');
        terminal_put_at(x, BOARD_TOP + BOARD_HEIGHT - 1, '#');
    }

    for (int y = BOARD_TOP; y < BOARD_TOP + BOARD_HEIGHT; y++) {
        terminal_put_at(BOARD_LEFT, y, '#');
        terminal_put_at(BOARD_LEFT + BOARD_WIDTH - 1, y, '#');
    }
}

// Draw the whole frame
static void render_frame(void) {
    char score_buf[16];

    // Clear the screen first
    terminal_clear();
    // Show simple game help
    draw_text(0, 0, "Snake: Arrow keys to move, ESC to quit");

    // Print the score at the top
    u32_to_dec(score, score_buf);
    draw_text(0, 1, "Score: ");
    draw_text(7, 1, score_buf);

    // Draw the board and the food
    draw_board();

    terminal_put_at(food.x, food.y, '*');

    // Draw the snake head and body
    for (uint32_t i = 0; i < snake_len; i++) {
        terminal_put_at(snake_cells[i].x, snake_cells[i].y, (i == 0u) ? '@' : 'o');
    }
}

// Check if the snake hits a wall
static bool hit_wall(point_t p) {
    return p.x <= BOARD_LEFT ||
           p.x >= (BOARD_LEFT + BOARD_WIDTH - 1) ||
           p.y <= BOARD_TOP ||
           p.y >= (BOARD_TOP + BOARD_HEIGHT - 1);
}

// Check if the snake hits its own body
static bool hit_body(point_t p, bool grow) {
    uint32_t check_len = snake_len;
    if (!grow && check_len > 0u) {
        check_len--;
    }

    for (uint32_t i = 0; i < check_len; i++) {
        if (point_eq(snake_cells[i], p)) {
            return true;
        }
    }

    return false;
}

// Change direction unless it would go backwards
static void change_direction(int nx, int ny) {
    if (dir_x == -nx && dir_y == -ny) {
        return;
    }
    dir_x = nx;
    dir_y = ny;
}

// Run the Snake game
bool snake_run(void) {
    uint32_t last_step;
    uint32_t step_ms = 140u;
    bool game_over = false;

    // Start with no keyboard echo
    kbd_set_echo(false);

    // Reset the snake and score
    snake_len = 4;
    score = 0;
    dir_x = 1;
    dir_y = 0;

    int start_x = BOARD_LEFT + (BOARD_WIDTH / 2);
    int start_y = BOARD_TOP + (BOARD_HEIGHT / 2);

    // Build the first snake body
    for (uint32_t i = 0; i < snake_len; i++) {
        snake_cells[i].x = start_x - (int)i;
        snake_cells[i].y = start_y;
    }

    // Place food and draw the first frame
    spawn_food();
    render_frame();

    // Start the loop song
    pcspk_bg_start(&snake_song, 1);

    last_step = pit_get_ticks();

    while (!game_over) {
        int c;
        uint32_t now;

        // Keep the music going
        pcspk_bg_update();

        // Read all ready keys
        while ((c = kbd_try_getchar()) != -1) {
            // ESC exits the game
            if (c == 27) {
                pcspk_bg_stop();
                terminal_clear();
                terminal_home();
                return true;
            }
            // Arrow keys control the snake
            if (c == KBD_KEY_UP) {
                change_direction(0, -1);
            } else if (c == KBD_KEY_DOWN) {
                change_direction(0, 1);
            } else if (c == KBD_KEY_LEFT) {
                change_direction(-1, 0);
            } else if (c == KBD_KEY_RIGHT) {
                change_direction(1, 0);
            }
        }

        // Wait for the next move tick
        now = pit_get_ticks();
        if ((uint32_t)(now - last_step) < step_ms) {
            __asm__ volatile ("sti; hlt");
            continue;
        }
        last_step = now;

        // Move the head one step forward
        point_t next_head = { snake_cells[0].x + dir_x, snake_cells[0].y + dir_y };
        // Check if food is eaten
        bool grow = point_eq(next_head, food);

        // End the game on collision
        if (hit_wall(next_head) || hit_body(next_head, grow)) {
            game_over = true;
            break;
        }

        // Grow and speed up after eating food
        if (grow && snake_len < SNAKE_MAX_CELLS) {
            snake_len++;
            score++;
            if (step_ms > 70u) {
                step_ms -= 2u;
            }
        }

        // Move the body forward
        for (uint32_t i = snake_len - 1; i > 0; i--) {
            snake_cells[i] = snake_cells[i - 1];
        }
        snake_cells[0] = next_head;

        // Spawn new food after eating
        if (grow) {
            spawn_food();
        }

        // Draw the updated frame
        render_frame();
    }

    // Stop the song when the game ends
    pcspk_bg_stop();

    // Wait for ENTER or ESC after game over
    draw_text(0, 23, "Game over. Press ENTER or ESC.");
    for (;;) {
        int c = kbd_getchar();
        if (c == 27 || c == '\n' || c == '\r') {
            break;
        }
    }

    terminal_clear();
    terminal_home();
    return true;
}
