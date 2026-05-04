#include "games/snake.h"
#include "libc/stdio.h"


static SnakeGame game;

static uint32_t snake_rand_state = 12345;

static uint32_t snake_rand(void) {
    snake_rand_state = snake_rand_state * 1103515245 + 12345;
    return snake_rand_state;
}

static void snake_place_food(void) {
    int valid = 0;

    while (!valid) {
        valid = 1;
        game.food_x = (int)(snake_rand() % SNAKE_BOARD_WIDTH);
        game.food_y = (int)(snake_rand() % SNAKE_BOARD_HEIGHT);

        for (int i = 0; i < game.length; i++) {
            if (game.x[i] == game.food_x && game.y[i] == game.food_y) {
                valid = 0;
                break;
            }
        }
    }
}

void snake_init(void) {
    for (int i = 0; i < SNAKE_MAX_LENGTH; i++) {
    
        game.x[i] = 0;
        game.y[i] = 0;
    }

    game.length = 0;
    game.food_x = 0;
    game.food_y = 0;
    game.direction = SNAKE_DIR_RIGHT;
    game.score = 0;
    game.game_over = 0;
    game.initialized = 0;

    game.length = 3;
    game.x[0] = SNAKE_BOARD_WIDTH / 2;
    game.y[0] = SNAKE_BOARD_HEIGHT / 2;

    game.x[1] = game.x[0] - 1;
    game.y[1] = game.y[0];

    game.x[2] = game.x[1] - 1;
    game.y[2] = game.y[1];

    game.direction = SNAKE_DIR_RIGHT;
    game.score = 0;
    game.game_over = 0;
    game.initialized = 1;

    snake_place_food();
}

static int snake_is_body(int x, int y) {
    for (int i = 0; i < game.length; i++) {
        if (game.x[i] == x && game.y[i] == y) {
            return i + 1;
        }
    }
    return 0;
}

void snake_draw(void) {
    printf("\033[2J");
    printf("\033[H");

    printf("Snake OS Game  |  Score: %d\n", game.score);

    for (int y = -1; y <= SNAKE_BOARD_HEIGHT; y++) {
        for (int x = -1; x <= SNAKE_BOARD_WIDTH; x++) {
            if (y == -1 || y == SNAKE_BOARD_HEIGHT || x == -1 || x == SNAKE_BOARD_WIDTH) {
                printf("#");
            } else if (x == game.food_x && y == game.food_y) {
                printf("*");
            } else {
                int body_index = snake_is_body(x, y);
                if (body_index == 1) {
                    printf("O");
                } else if (body_index > 1) {
                    printf("o");
                } else {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }

    if (game.game_over) {
        printf("\nGAME OVER! Final score: %d\n", game.score);
        printf("Stop the kernel or add restart later.\n");
    } else {
        printf("\nUse W A S D to move.\n");
    }
}

void snake_handle_key(char key) {
    if (!game.initialized || game.game_over) {
        return;
    }

    if ((key == 'w' || key == 'W') && game.direction != SNAKE_DIR_DOWN) {
        game.direction = SNAKE_DIR_UP;
    } else if ((key == 's' || key == 'S') && game.direction != SNAKE_DIR_UP) {
        game.direction = SNAKE_DIR_DOWN;
    } else if ((key == 'a' || key == 'A') && game.direction != SNAKE_DIR_RIGHT) {
        game.direction = SNAKE_DIR_LEFT;
    } else if ((key == 'd' || key == 'D') && game.direction != SNAKE_DIR_LEFT) {
        game.direction = SNAKE_DIR_RIGHT;
    }
}

void snake_update(void) {
    if (!game.initialized || game.game_over) {
        return;
    }

    int new_x = game.x[0];
    int new_y = game.y[0];

    if (game.direction == SNAKE_DIR_UP) {
        new_y--;
    } else if (game.direction == SNAKE_DIR_DOWN) {
        new_y++;
    } else if (game.direction == SNAKE_DIR_LEFT) {
        new_x--;
    } else if (game.direction == SNAKE_DIR_RIGHT) {
        new_x++;
    }

    if (new_x < 0 || new_x >= SNAKE_BOARD_WIDTH || new_y < 0 || new_y >= SNAKE_BOARD_HEIGHT) {
        game.game_over = 1;
        return;
    }

    for (int i = 0; i < game.length; i++) {
        if (game.x[i] == new_x && game.y[i] == new_y) {
            game.game_over = 1;
            return;
        }
    }

    int ate_food = (new_x == game.food_x && new_y == game.food_y);

    if (ate_food) {
        if (game.length < SNAKE_MAX_LENGTH) {
            game.length++;
        }
        game.score++;
    }

    for (int i = game.length - 1; i > 0; i--) {
        game.x[i] = game.x[i - 1];
        game.y[i] = game.y[i - 1];
    }

    game.x[0] = new_x;
    game.y[0] = new_y;

    if (ate_food) {
        snake_place_food();
    }
}

int snake_is_running(void) {
    return game.initialized && !game.game_over;
}
