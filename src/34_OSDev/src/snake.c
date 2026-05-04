#include "snake.h"
#include "terminal.h"
#include "pit.h"
#include <libc/stdio.h>

//VGA buffer so we can write directly to specific screen positions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((uint16_t*)0xB8000)

//global direction variable set by keyboard interrupt
volatile int snake_direction = DIR_RIGHT;
volatile int direction_changed = 0;

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

//write a character at a specific x,y position on screen
static void put_char(int x, int y, char c, uint8_t color) {
    VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(c, color);
}

//clear the whole screen
static void clear_screen(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            put_char(x, y, ' ', 0x00);
        }
    }
}

//simple random number using tick count, not great but works
static uint32_t rand_seed = 12345;
static uint32_t simple_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed >> 16) & 0x7FFF;
}

//place food at a random position that isnt on the snake
static void spawn_food(Game *game) {
    int valid = 0;
    while (!valid) {
        game->food.x = simple_rand() % GAME_WIDTH;
        game->food.y = (simple_rand() % (GAME_HEIGHT - 2)) + 2; //keep top 2 rows for score
        valid = 1;
        for (int i = 0; i < game->snake.length; i++) {
            if (game->snake.body[i].x == game->food.x &&
                game->snake.body[i].y == game->food.y) {
                valid = 0;
                break;
            }
        }
    }
}

//draw everything to screen
static void draw_game(Game *game) {
    clear_screen();

    //draw score bar at the top
    char buf[40];
    int len = 0;
    buf[len++] = 'S';
    buf[len++] = 'c';
    buf[len++] = 'o';
    buf[len++] = 'r';
    buf[len++] = 'e';
    buf[len++] = ':';
    buf[len++] = ' ';

    //convert score to string
    int score = game->score;
    if (score == 0) {
        buf[len++] = '0';
    } else {
        char digits[10];
        int d = 0;
        while (score > 0) {
            digits[d++] = '0' + (score % 10);
            score /= 10;
        }
        for (int i = d - 1; i >= 0; i--) {
            buf[len++] = digits[i];
        }
    }
    buf[len] = '\0';

    for (int i = 0; i < len; i++) {
        put_char(i, 0, buf[i], 0x0F);
    }

    //draw border line under score
    for (int x = 0; x < GAME_WIDTH; x++) {
        put_char(x, 1, '-', 0x07);
    }

    //draw food
    put_char(game->food.x, game->food.y, '*', 0x0E);

    //draw snake body
    for (int i = 1; i < game->snake.length; i++) {
        put_char(game->snake.body[i].x, game->snake.body[i].y, 'o', 0x0A);
    }

    //draw snake head
    put_char(game->snake.body[0].x, game->snake.body[0].y, 'O', 0x0A);
}

//draw game over message in the middle of screen
static void draw_game_over(Game *game) {
    const char *msg = "GAME OVER";
    int start_x = (GAME_WIDTH - 9) / 2;
    int y = GAME_HEIGHT / 2;
    for (int i = 0; i < 9; i++) {
        put_char(start_x + i, y, msg[i], 0x0C);
    }

    const char *msg2 = "Press R to restart";
    int start_x2 = (GAME_WIDTH - 18) / 2;
    for (int i = 0; i < 18; i++) {
        put_char(start_x2 + i, y + 1, msg2[i], 0x07);
    }
}

//called from keyboard IRQ handler in irq.c
void snake_key_handler(uint8_t scancode) {
    //only allow direction change once per tick to prevent 180 degree turns
    if (direction_changed) return;

    switch (scancode) {
        case 0x11: //W
            if (snake_direction != DIR_DOWN) {
                snake_direction = DIR_UP;
                direction_changed = 1;
            }
            break;
        case 0x1F: //S
            if (snake_direction != DIR_UP) {
                snake_direction = DIR_DOWN;
                direction_changed = 1;
            }
            break;
        case 0x1E: //A
            if (snake_direction != DIR_RIGHT) {
                snake_direction = DIR_LEFT;
                direction_changed = 1;
            }
            break;
        case 0x20: //D
            if (snake_direction != DIR_LEFT) {
                snake_direction = DIR_RIGHT;
                direction_changed = 1;
            }
            break;
    }
}

static void init_game(Game *game) {
    game->score = 0;
    game->tick_rate = 150; //ms between each move
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;
    game->snake.alive = 1;

    //start snake in the middle of screen
    int start_x = GAME_WIDTH / 2;
    int start_y = GAME_HEIGHT / 2;
    for (int i = 0; i < game->snake.length; i++) {
        game->snake.body[i].x = start_x - i;
        game->snake.body[i].y = start_y;
    }

    snake_direction = DIR_RIGHT;
    direction_changed = 0;
    rand_seed = get_tick_count(); //seed random with current time
    spawn_food(game);
}

static void update_game(Game *game) {
    //move body segments forward (last follows second to last, etc)
    for (int i = game->snake.length - 1; i > 0; i--) {
        game->snake.body[i] = game->snake.body[i - 1];
    }

    //move head in current direction
    game->snake.direction = snake_direction;
    switch (game->snake.direction) {
        case DIR_UP:    game->snake.body[0].y--; break;
        case DIR_DOWN:  game->snake.body[0].y++; break;
        case DIR_LEFT:  game->snake.body[0].x--; break;
        case DIR_RIGHT: game->snake.body[0].x++; break;
    }

    direction_changed = 0;

    //die if snake hits the border
    if (game->snake.body[0].x < 0 || game->snake.body[0].x >= GAME_WIDTH ||
        game->snake.body[0].y < 2 || game->snake.body[0].y >= GAME_HEIGHT) {
        game->snake.alive = 0;
        return;
    }

    //check if snake hit itself
    for (int i = 1; i < game->snake.length; i++) {
        if (game->snake.body[0].x == game->snake.body[i].x &&
            game->snake.body[0].y == game->snake.body[i].y) {
            game->snake.alive = 0;
            return;
        }
    }

    //check if snake ate food
    if (game->snake.body[0].x == game->food.x &&
        game->snake.body[0].y == game->food.y) {
        if (game->snake.length < MAX_SNAKE_LENGTH) {
            game->snake.length++;
        }
        game->score += 10;
        spawn_food(game);

        //speed up slightly every 50 points
        if (game->tick_rate > 60 && game->score % 50 == 0) {
            game->tick_rate -= 10;
        }
    }
}

//restart flag set by keyboard handler
volatile int restart_pressed = 0;

void snake_restart_handler(uint8_t scancode) {
    if (scancode == 0x13) { //R key
        restart_pressed = 1;
    }
}

void snake_game(void) {
    Game game;

    while (1) {
        init_game(&game);
        draw_game(&game);

        //main game loop
        while (game.snake.alive) {
            sleep_interrupt(game.tick_rate);
            update_game(&game);
            draw_game(&game);
        }

        draw_game_over(&game);

        //wait for R key to restart
        restart_pressed = 0;
        while (!restart_pressed) {
            sleep_interrupt(100);
        }
    }
}
