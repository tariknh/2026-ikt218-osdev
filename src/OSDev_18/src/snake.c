#include <snakeApp/snake.h>
#include <kernel/memory.h>
#include <kernel/pit.h>
#include <kernel/keyboard.h>
#include <kernel/terminal.h>
#include <songApp/song.h>
#include <songApp/frequencies.h>

struct GameState* CreateGame(void) {
    struct GameState* game = (struct GameState*)malloc(sizeof(struct GameState));
    if (!game) {
        return 0;
    }

    game->snake = (struct Snake*)malloc(sizeof(struct Snake));
    if (!game->snake) {
        free(game);
        return 0;
    }

    game->food = (struct Food*)malloc(sizeof(struct Food));
    if (!game->food) {
        free(game->snake);
        free(game);
        return 0;
    }

    game->score = 0;
    game->rngState = GetCurrentTick();

    InitializeBoard(game);
    InitializeSnake(game->snake);
    InitializeFood(game->food);

    return game;
}

void DestroyGame(struct GameState* game) {
    if (!game) return;

    if (game->snake) {
        free(game->snake);
    }

    if (game->food) {
        free(game->food);
    }
    
    free(game);
}

void ResetGame(struct GameState* game) {
    if (!game) return;

    InitializeBoard(game);
    InitializeSnake(game->snake);
    InitializeFood(game->food);
    game->score = 0;
    game->rngState = GetCurrentTick();
}

void InitializeBoard(struct GameState* game) {
    for (uint32_t i = 0; i < BOARD_SIZE; i++) {
        for (uint32_t j = 0; j < BOARD_SIZE; j++) {
            game->board[i][j][0] = ' ';
            game->board[i][j][1] = ' ';
            game->board[i][j][2] = '\0';
        }
    }
}

void InitializeSnake(struct Snake* snake) {
    snake->length = 1;
    snake->direction = LEFT;
    snake->body[0].x = BOARD_SIZE / 2;
    snake->body[0].y = BOARD_SIZE / 2;
}

void InitializeFood(struct Food* food) {
    food->x = (BOARD_SIZE / 2) - 3;
    food->y = (BOARD_SIZE / 2) - 3;
}

uint32_t Random(uint32_t* rngState) {
    *rngState = (*rngState * 1103515245) + 12345;
    return *rngState;
}

void HandleInput(struct GameState* game, char input) {
    switch (input) {
        case 'w': {
            if (game->snake->direction != DOWN) {
                game->snake->direction = UP;
            }
            break;
        }
        case 's': {
            if (game->snake->direction != UP) {
                game->snake->direction = DOWN;
            }
            break;
        }
        case 'a': {
            if (game->snake->direction != RIGHT) {
                game->snake->direction = LEFT;
            }
            break;
        }
        case 'd': {
            if (game->snake->direction != LEFT) {
                game->snake->direction = RIGHT;
            }
            break;
        }
        case 'r': {
            if (game) {
                ResetGame(game);
            }
            break;
        }
        case 'q': {
            TerminalWriteString("Quitting game...\n");
            DestroyGame(game);
        }
        default: {
            break;
        }
    }
}

void PlayFoodSound(void) {
    PlaySound(B5);
    SleepInterrupt(50);
    StopSound();
}

void PlayDeathSound(void) {
    PlaySound(G3);
    SleepInterrupt(100);
    StopSound();
}

void PlayWinSound(void) {
    PlaySound(C5); 
    SleepInterrupt(100); 
    StopSound();

    PlaySound(E5); 
    SleepInterrupt(100); 
    StopSound();

    PlaySound(G5); 
    SleepInterrupt(200); 
    StopSound();
}

struct SnakeSegment MoveSnake(struct Snake* snake) {
    int x = snake->body[0].x;
    int y = snake->body[0].y;

    // Move the head first, then propagate the old position through the rest
    // of the body. The returned segment is the previous tail position so the
    // caller can grow the snake there after eating food.
    switch (snake->direction) {
        case UP: {
            snake->body[0].y--;
            for (uint32_t i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case DOWN: {
            snake->body[0].y++;
            for (uint32_t i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case LEFT: {
            snake->body[0].x--;
            for (uint32_t i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        case RIGHT: {
            snake->body[0].x++;
            for (uint32_t i = 1; i < snake->length; i++) {
                int tempX = snake->body[i].x;
                int tempY = snake->body[i].y;
                snake->body[i].x = x;
                snake->body[i].y = y;
                x = tempX;
                y = tempY;
            }
            break;
        }
        default: {
            break;
        }
    }
    return (struct SnakeSegment){ x, y };
}

void SpawnFood(struct GameState* game) {
    int x, y;
    uint32_t occupied;

    if (game->snake->length == SNAKE_MAX_LENGTH) {
        return;
    }

    do {
        x = Random(&game->rngState) % BOARD_SIZE;
        y = Random(&game->rngState) % BOARD_SIZE;

        occupied = 0;

        for (uint32_t i = 0; i < game->snake->length; i++) {
            if (game->snake->body[i].x == x && game->snake->body[i].y == y) {
                occupied = 1;
                break;
            }
        }
    } while (occupied);

    game->food->x = x;
    game->food->y = y;
}

void AddSegment(struct Snake* snake, int x, int y) {
    if (snake->length < SNAKE_MAX_LENGTH) {
        snake->body[snake->length].x = x;
        snake->body[snake->length].y = y;
        snake->length++;
    }
}

enum CollisionType CheckCollision(struct Snake* snake, struct Food* food) {
    if (snake->body[0].x == food->x && snake->body[0].y == food->y) {
        return FOOD;
    } else if (snake->body[0].x < 0 || snake->body[0].x >= BOARD_SIZE || snake->body[0].y < 0 || snake->body[0].y >= BOARD_SIZE) {
        return WALL;
    } else {
        for (uint32_t i = 1; i < snake->length; i++) {
            if (snake->body[0].x == snake->body[i].x && snake->body[0].y == snake->body[i].y) {
                return SELF;
            }
        }
        return NONE;
    }
}

void DrawBoard(struct GameState* game) {
    TerminalClear();
    InitializeBoard(game);

    if (game->snake->direction == UP) {
        game->board[game->snake->body[0].y][game->snake->body[0].x][0] = '^';
        game->board[game->snake->body[0].y][game->snake->body[0].x][1] = '^';
    } else if (game->snake->direction == DOWN) {
        game->board[game->snake->body[0].y][game->snake->body[0].x][0] = 'v';
        game->board[game->snake->body[0].y][game->snake->body[0].x][1] = 'v';
    } else if (game->snake->direction == LEFT) {
        game->board[game->snake->body[0].y][game->snake->body[0].x][0] = '<';
        game->board[game->snake->body[0].y][game->snake->body[0].x][1] = '<';
    } else if (game->snake->direction == RIGHT) {
        game->board[game->snake->body[0].y][game->snake->body[0].x][0] = '>';
        game->board[game->snake->body[0].y][game->snake->body[0].x][1] = '>';
    }

    for (uint32_t i = 1; i < game->snake->length; i++) {
        game->board[game->snake->body[i].y][game->snake->body[i].x][0] = '[';
        game->board[game->snake->body[i].y][game->snake->body[i].x][1] = ']';
    }

    game->board[game->food->y][game->food->x][0] = '{';
    game->board[game->food->y][game->food->x][1] = '}';

    for (uint32_t i = 0; i < BOARD_SIZE + 2; i++) {
        TerminalWriteString("##");
    }
    TerminalPutChar('\n');

    for (uint32_t i = 0; i < BOARD_SIZE; i++) {
        TerminalWriteString("##");
        for (uint32_t j = 0; j < BOARD_SIZE; j++) {
            TerminalWriteString(game->board[i][j]);
        }
        TerminalWriteString("##\n");
    }

    for (uint32_t i = 0; i < BOARD_SIZE + 2; i++) {
        TerminalWriteString("##");
    }
    TerminalPutChar('\n');

    TerminalWriteString("Score: ");
    TerminalWriteUInt(game->score);
    TerminalPutChar('\n');

    TerminalWriteString("Press q to exit\n");
}

void PlayGame(void) {
    struct GameState* game = CreateGame();
    if (!game) {
        TerminalWriteString("Failed to create game.\n");
        return;
    }

    char input = 0;
    struct SnakeSegment tail;
    enum CollisionType collisionType = NONE;

    while(1) {
        if (game->snake->length == SNAKE_MAX_LENGTH) {
            break;
        }
        
        input = GetLastKeyPressed();
        HandleInput(game, input);
        if (input == 'q') return;
        tail = MoveSnake(game->snake);

        collisionType = CheckCollision(game->snake, game->food);
        if (collisionType == FOOD) {
            game->score++;
            PlayFoodSound();
            AddSegment(game->snake, tail.x, tail.y);
            SpawnFood(game);
        } else if (collisionType == WALL || collisionType == SELF) {
            PlayDeathSound();
            ResetGame(game);
        }

        collisionType = NONE;

        DrawBoard(game);

        SleepInterrupt(GAME_SPEED_MS);
    }

    TerminalWriteString("You win!\n");
    PlayWinSound();
    DestroyGame(game);
}
