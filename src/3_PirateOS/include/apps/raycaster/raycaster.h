#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "libc/stdint.h"
#include "libc/stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// Size of the 2D map used for collisions/walls
#define MAP_WIDTH 16
#define MAP_HEIGHT 16

// View settings
#define FOV 60  // Field of view in degrees
#define MAX_DEPTH 20

// Player position and look direction
typedef struct {
    double x, y; // Position in map
    double angle; // Viewing angle in degrees (0-360)
    double speed; // Movement speed
} Player;

// Main game state
typedef struct {
    Player player;
    uint32_t frame_count; // Number of rendered frames
    bool game_running; // False means exit game loop
} Raycaster;

// Core game functions
// Set default values for the game state
void raycaster_init(Raycaster *rc);

// Returns 1 if position hits a wall, 0 if it is free
int raycaster_check_collision(double x, double y);

// Runs the full raycaster game loop
void raycaster_game_loop(void);

// Input hooks used by keyboard IRQ and kernel main loop
// Mark raycaster input as active or inactive
void raycaster_input_set_active(int enabled);

// Returns 1 when raycaster input is active
int raycaster_input_is_active(void);

// Queue one keyboard scancode for the game
void raycaster_input_submit_scancode(uint8_t scancode);

// Ask kernel loop to launch the game on next tick
void raycaster_input_request_launch(void);

// Returns and clears pending launch request
uint8_t raycaster_input_consume_launch_request(void);

// Ask game loop to exit as soon as possible
void raycaster_input_request_exit(void);

#ifdef __cplusplus
}
#endif

#endif
