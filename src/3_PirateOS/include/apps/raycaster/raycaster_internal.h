#ifndef APPS_RAYCASTER_INTERNAL_H
#define APPS_RAYCASTER_INTERNAL_H

#include "apps/raycaster/raycaster.h"

// Indexes into the key_down[] array
#define RC_KEY_W 0
#define RC_KEY_S 1
#define RC_KEY_A 2
#define RC_KEY_D 3
#define RC_KEY_Q 4
#define RC_KEY_E 5
#define RC_KEY_COUNT 6

// Bit flags for held keys
#define GAME_KEY_W (1u << 0)
#define GAME_KEY_S (1u << 1)
#define GAME_KEY_A (1u << 2)
#define GAME_KEY_D (1u << 3)
#define GAME_KEY_Q (1u << 4)
#define GAME_KEY_E (1u << 5)

// Movement/physics settings
#define RAYCASTER_TURN_STEP_DEG 3.0
#define RAYCASTER_MOVE_STEP 0.04
#define RAYCASTER_PHYSICS_STEP_MS 16u

// Shared map data (defined in core file)
extern const uint8_t g_raycaster_map_data[MAP_HEIGHT][MAP_WIDTH];

// Math helpers
// Returns sin(angle) where angle is in degrees.
double raycaster_math_sin(double angle);
// Returns cos(angle) where angle is in degrees.
double raycaster_math_cos(double angle);

// Input and movement helpers
// Applies current key state to player movement/rotation
void raycaster_apply_key_state_internal(Raycaster *rc, uint8_t *key_down);
// Reads current controls and fills key_down array
char raycaster_poll_controls_internal(Raycaster *rc, uint8_t *key_down);
// Pops one queued scancode; returns 1 if one was available
uint8_t raycaster_input_pop_scancode_internal(uint8_t *scancode);
// Returns and clears pending exit request
uint8_t raycaster_input_consume_exit_request_internal(void);

// VGA rendering helpers
// Switches VGA to mode 13h (320x200x256)
void raycaster_vga13_set_mode_internal(void);
// Restores VGA text mode after the game exits
void raycaster_vga_text_set_mode_internal(void);
// Clears the backbuffer with one color
void raycaster_vga13_clear_internal(uint8_t color);
// Renders the 3D scene to the backbuffer
void raycaster_render_mode13_internal(Raycaster *rc);
// Draws the top-down minimap overlay
void raycaster_draw_minimap_mode13_internal(Raycaster *rc);
// Copies backbuffer to VGA memory (shows current frame)
void raycaster_vga13_present_internal(void);

#endif
