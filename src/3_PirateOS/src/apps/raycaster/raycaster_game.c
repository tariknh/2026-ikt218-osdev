#include "apps/raycaster/raycaster_internal.h"
#include "apps/song/song.h"
#include "apps/song/song_data.h"
#include "arch/i386/io.h"
#include "kernel/pit.h"
#include "memory/heap.h"

static void raycaster_play_note_internal(const Note *note)
{
    // R means rest so speaker should be silent
    if (note->frequency == R) {
        stop_sound();
        return;
    }

    play_sound(note->frequency);
}

// Initialize and play the first note in the loop
static void raycaster_start_background_theme(uint32_t current_ticks, uint32_t *current_note_index, uint32_t *note_end_tick)
{
    const uint32_t bg_theme_len = raycaster_bg_theme_length;
    if (bg_theme_len == 0) {
        return;
    }

    *current_note_index = 0;
    const Note *note = &raycaster_bg_theme[*current_note_index];
    raycaster_play_note_internal(note);
    *note_end_tick = current_ticks + note->duration;
}

// Advance song timing without blocking render/update. Always wraps to start
static void raycaster_update_background_theme(uint32_t current_ticks, uint32_t *current_note_index, uint32_t *note_end_tick)
{
    const uint32_t bg_theme_len = raycaster_bg_theme_length;
    if (bg_theme_len == 0) {
        return;
    }

    // If we lagged behind, advance through as many notes as needed
    while ((int32_t)(current_ticks - *note_end_tick) >= 0) {
        // Move to next note and wrap when the song reaches the end
        *current_note_index = (*current_note_index + 1U) % bg_theme_len;
        const Note *note = &raycaster_bg_theme[*current_note_index];
        raycaster_play_note_internal(note);
        *note_end_tick += note->duration;
    }
}

// Main game loop: input -> update -> render -> present
void raycaster_game_loop(void)
{
    // ~60 FPS target frame time
    const uint32_t target_frame_ms = 16;
    uint32_t music_index = 0;
    uint32_t next_note_tick = 0;
    // Allocate the main game state on the heap
    Raycaster *raycaster = (Raycaster *)malloc(sizeof(Raycaster));
    // Store current held-key state on the heap
    uint8_t *key_down = (uint8_t *)malloc(RC_KEY_COUNT);

    if (!raycaster || !key_down) {
        // Clean up whichever allocation succeeded before leaving early
        free(key_down);
        free(raycaster);
        return;
    }

    // Start with no keys pressed
    for (uint32_t i = 0; i < RC_KEY_COUNT; i++) {
        key_down[i] = 0;
    }

    // Enter game mode: capture input + switch to VGA graphics mode
    raycaster_input_set_active(1);
    raycaster_vga13_set_mode_internal();
    raycaster_vga13_clear_internal(0);

    // Create and initialize game state.
    raycaster_init(raycaster);

    // Restart background music from the first note for each game launch
    stop_sound();
    music_index = 0;
    raycaster_start_background_theme(pit_get_ticks(), &music_index, &next_note_tick);

    // Fixed-step physics timer state
    uint32_t last_physics_tick = pit_get_ticks();
    uint32_t physics_accumulator = 0;

    // Minimap starts visible, this tracks M key debounce
    int minimap_visible = 1;
    uint8_t minimap_m_down = 0;

    while (raycaster->game_running) {
        int exit_now = 0;

        // Used to limit frame duration at end of loop
        uint32_t frame_start_ticks = pit_get_ticks();

        // Poll current controls (WASD/QE + instant keys)
        char instant_key = raycaster_poll_controls_internal(raycaster, key_down);
        if (!raycaster->game_running) {
            stop_sound();
            break;
        }

        // Exit requested by keyboard handler
        if (raycaster_input_consume_exit_request_internal()) {
            raycaster->game_running = false;
            exit_now = 1;
            stop_sound();
        }
        uint32_t current_ticks = pit_get_ticks();
        uint8_t scancode = 0;

        // Consume all pending raw scancodes for one-shot actions
        while (raycaster_input_pop_scancode_internal(&scancode)) {
            uint8_t code = (uint8_t)(scancode & 0x7F);
            uint8_t released = (uint8_t)(scancode & 0x80);

            // ESC exits the game immediately
            if (code == 0x01) {
                raycaster->game_running = false;
                exit_now = 1;
                stop_sound();
                break;
            }

            // M toggles minimap only on key press (not while held down)
            if (code == 0x32) {
                if (!released) {
                    if (!minimap_m_down) {
                        minimap_visible = !minimap_visible;
                        minimap_m_down = 1;
                    }
                } else {
                    minimap_m_down = 0;
                }
            }
        }

        if (exit_now) {
            // Skip update and render work when exit was requested this frame
            break;
        }

        // Add elapsed real time to fixed-step physics accumulator
        uint32_t elapsed_ms = current_ticks - last_physics_tick;
        last_physics_tick = current_ticks;
        // Clamp big jumps so movement stays predictable after lag spikes
        if (elapsed_ms > 100) elapsed_ms = 100;
        physics_accumulator += elapsed_ms;

        // Instant keys should trigger at least one physics step
        if (instant_key == 'w' || instant_key == 'W') key_down[RC_KEY_W] = 1;
        else if (instant_key == 's' || instant_key == 'S') key_down[RC_KEY_S] = 1;
        else if (instant_key == 'a' || instant_key == 'A') key_down[RC_KEY_A] = 1;
        else if (instant_key == 'd' || instant_key == 'D') key_down[RC_KEY_D] = 1;
        else if (instant_key == 'q' || instant_key == 'Q') key_down[RC_KEY_Q] = 1;
        else if (instant_key == 'e' || instant_key == 'E') key_down[RC_KEY_E] = 1;

        if (instant_key != 0 && physics_accumulator < RAYCASTER_PHYSICS_STEP_MS) {
            // Force one physics step so tap input feels responsive
            physics_accumulator = RAYCASTER_PHYSICS_STEP_MS;
        }

        // Run physics in fixed-size time slices for stable movement
        while (physics_accumulator >= RAYCASTER_PHYSICS_STEP_MS) {
            raycaster->player.speed = RAYCASTER_MOVE_STEP;

            if (key_down[RC_KEY_W] || key_down[RC_KEY_S] || key_down[RC_KEY_A] ||
                key_down[RC_KEY_D] || key_down[RC_KEY_Q] || key_down[RC_KEY_E]) {
                raycaster_apply_key_state_internal(raycaster, key_down);
            }

            physics_accumulator -= RAYCASTER_PHYSICS_STEP_MS;
        }

        raycaster->frame_count++;

        // Draw world, optional minimap, then show frame
        raycaster_render_mode13_internal(raycaster);
        if (minimap_visible) {
            raycaster_draw_minimap_mode13_internal(raycaster);
        }
        raycaster_vga13_present_internal();

        // Update music after rendering
        current_ticks = pit_get_ticks();
        raycaster_update_background_theme(current_ticks, &music_index, &next_note_tick);

        // Sleep the remaining frame time to keep FPS stable
        {
            uint32_t frame_time = pit_get_ticks() - frame_start_ticks;
            if (frame_time < target_frame_ms) {
                sleep_interrupt(target_frame_ms - frame_time);
            }
        }
    }

    // Leave game mode and restore terminal environment
    stop_sound();
    raycaster_vga_text_set_mode_internal();
    raycaster_input_set_active(0);
    // Release heap memory used by this game session
    free(key_down);
    free(raycaster);
}
