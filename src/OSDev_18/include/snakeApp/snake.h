#ifndef SNAKEAPP_SNAKE_H
#define SNAKEAPP_SNAKE_H

#include <libc/stdint.h>

#define BOARD_SIZE 15
#define SNAKE_MAX_LENGTH (BOARD_SIZE * BOARD_SIZE)
#define GAME_SPEED_MS 500

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

enum CollisionType {
    NONE,
    FOOD,
    SELF,
    WALL
};

struct SnakeSegment {
    int x;
    int y;
};

struct Snake {
    uint32_t length;
    enum Direction direction;
    struct SnakeSegment body[SNAKE_MAX_LENGTH];
};

struct Food {
    int x;
    int y;
};

struct GameState {
    char board[BOARD_SIZE][BOARD_SIZE][3];
    struct Snake* snake;
    struct Food* food;
    uint32_t score;
    uint32_t rngState;
};

struct GameState* CreateGame(void);
void DestroyGame(struct GameState* game);
void ResetGame(struct GameState* game);

void InitializeBoard(struct GameState* game);
void InitializeSnake(struct Snake* snake);
void InitializeFood(struct Food* food);

uint32_t Random(uint32_t* rngState);
void HandleInput(struct GameState* game, char input);

void PlayFoodSound(void);
void PlayDeathSound(void);
void PlayWinSound(void);

struct SnakeSegment MoveSnake(struct Snake* snake);
void SpawnFood(struct GameState* game);
void AddSegment(struct Snake* snake, int x, int y);
enum CollisionType CheckCollision(struct Snake* snake, struct Food* food);

void DrawBoard(struct GameState* game);

void PlayGame(void);

#endif