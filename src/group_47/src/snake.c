#include "snake.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "song.h"
#include "io.h"

// ===== Global Variables =====

snake_t snake;
volatile direction_t snake_dir;

static int food_x;
static int food_y;
static int game_over = 0;
static int base_delay = 150; 
static int score = 0;

// ===== Helper Functions =====

// Simple random food spawner (not really LOL)
static void spawn_food() {
    food_x = (food_x + 7) % SNAKE_WIDTH;
    food_y = (food_y + 5) % SNAKE_HEIGHT;
}

// Function to adjust snake speed accordingly to its length
void snake_set_speed() {
    int delay_ms = base_delay - ((snake.length * 3) * 1); 
    if (delay_ms < 40) delay_ms = 40; // minimum delay to prevent it from becoming unplayable
    sleep_interrupt(delay_ms);
}

// Function to play a sound effect for eating food
void play_tone(uint32_t frequency, int duration_ms) {
    enable_speaker();
    play_sound(frequency);
    sleep_interrupt(duration_ms);
    disable_speaker();
}

// ===== UI =====
void display_score() {
    printf("Score: ");
    print_int(score);
    printf("\n");
}

void display_home_screen() {
    terminal_clear();
    printf("===== Welcome to snake game =====\n");
    printf("Use arrow keys to control the snake.\n");
    printf("Press SPACE to start...\n");
    while (1) {
        if (is_key_pressed()) {
            char key = get_ascii_key();
            if (key == ' ') {
                break;
            }
        }
        sleep_interrupt(50);
    }
}

void display_game_over_screen() {
    terminal_clear();
    printf("===== GAME OVER =====\n");
    printf("Final score: ");
    print_int(score);
    printf("\n");
    printf("Press SPACE to play again...\n");
    while (1) {
        if (is_key_pressed()) {
            char key = get_ascii_key();
            if (key == ' ') {
                break;
            }
        }
        sleep_interrupt(50);
    }
}


// ===== Game Logic =====

// Initializes the snake in the middle of the board with a length of 3 and spawns the first food objects
static void init_game() {
    snake.length = 3;

    snake.x[0] = 10;
    snake.y[0] = 10;

    snake.x[1] = 9;
    snake.y[1] = 10;

    snake.x[2] = 8;
    snake.y[2] = 10;

    snake.dir = DIR_RIGHT;
    snake_dir = DIR_RIGHT;

    spawn_food();
}

// Sets the snake's direction, preventing it from reversing into itself
void snake_set_direction(direction_t dir) {
    // prevent reversing (important!)
    if ((snake.dir == DIR_UP && dir == DIR_DOWN) ||
        (snake.dir == DIR_DOWN && dir == DIR_UP) ||
        (snake.dir == DIR_LEFT && dir == DIR_RIGHT) ||
        (snake.dir == DIR_RIGHT && dir == DIR_LEFT)) {
        return;
    }
    snake_dir = dir;
}
// Updates the game logic - moves the snake, checks for collisions, and handles food consumption
static void update_snake() {
    // apply latest input
    snake.dir = snake_dir;

    // Logic to move body
    for (int i = snake.length - 1; i > 0; i--) {
        snake.x[i] = snake.x[i - 1];
        snake.y[i] = snake.y[i - 1];
    }

    // Logic to move head
    if (snake.dir == DIR_UP)    snake.y[0]--;
    if (snake.dir == DIR_RIGHT) snake.x[0]++;
    if (snake.dir == DIR_DOWN)  snake.y[0]++;
    if (snake.dir == DIR_LEFT)  snake.x[0]--;

    // Wall collision handler
    if (snake.x[0] < 0 || snake.x[0] >= SNAKE_WIDTH ||
        snake.y[0] < 0 || snake.y[0] >= SNAKE_HEIGHT) {
        game_over = 1;
    }

    // Self collision handler
    for (int i = 1; i < snake.length; i++) {
        if (snake.x[0] == snake.x[i] &&
            snake.y[0] == snake.y[i]) {
            game_over = 1;
        }
    }

    // Food consumption handler
    if (snake.x[0] == food_x && snake.y[0] == food_y) {
        if (snake.length < MAX_SNAKE_LENGTH) {
            snake.length++;
            score += 10;
        }
        play_tone(1000, 100);
        spawn_food();
    }
}

// ===== Drawing =====

static void draw() {
    terminal_clear();

    // draw snake
    for (int i = 0; i < snake.length; i++) {
        terminal_draw_entry('O', 0x0F, snake.x[i], snake.y[i]);
    }
    // draw food
    terminal_draw_entry('*', 0x0F, food_x, food_y);

    // draw score
    display_score();
}

// ===== Game Loop =====

void snake_start() {
    while (1) {
        display_home_screen();
        init_game();
        score = 0;
        game_over = 0;

        while (!game_over) {
            update_snake();
            draw();
            snake_set_speed();
        }
        display_game_over_screen();

        static int quit = 0;
        if (quit) {
            break;
        }
    }
}