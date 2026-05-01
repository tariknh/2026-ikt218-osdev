#include "apps/raycaster/raycaster_internal.h"

// Apply current input state to player movement and rotation
void raycaster_apply_key_state_internal(Raycaster *rc, uint8_t *key_down)
{
    // Build axis values from key states
    int move_axis = (key_down[RC_KEY_W] ? 1 : 0) - (key_down[RC_KEY_S] ? 1 : 0);
    int strafe_axis = (key_down[RC_KEY_D] ? 1 : 0) - (key_down[RC_KEY_A] ? 1 : 0);
    int turn_axis = (key_down[RC_KEY_E] ? 1 : 0) - (key_down[RC_KEY_Q] ? 1 : 0);

    // Rotate player left or right
    if (turn_axis != 0) {
        rc->player.angle += turn_axis * RAYCASTER_TURN_STEP_DEG;
        // Keep angle inside 0 to 360
        while (rc->player.angle < 0) rc->player.angle += 360;
        while (rc->player.angle >= 360) rc->player.angle -= 360;
    }

    // No movement requested
    if (move_axis == 0 && strafe_axis == 0) {
        return;
    }

    // Start from current position
    double new_x = rc->player.x;
    double new_y = rc->player.y;

    // Move forward or backward in facing direction
    if (move_axis != 0) {
        new_x += raycaster_math_cos(rc->player.angle) * rc->player.speed * move_axis;
        new_y += raycaster_math_sin(rc->player.angle) * rc->player.speed * move_axis;
    }

    // Strafe left or right relative to facing direction
    if (strafe_axis != 0) {
        new_x += raycaster_math_cos(rc->player.angle + 90) * rc->player.speed * strafe_axis;
        new_y += raycaster_math_sin(rc->player.angle + 90) * rc->player.speed * strafe_axis;
    }

    // Apply new position only if not colliding with a wall
    if (!raycaster_check_collision(new_x, new_y)) {
        rc->player.x = new_x;
        rc->player.y = new_y;
    }
}
