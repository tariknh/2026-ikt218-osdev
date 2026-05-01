#include "apps/raycaster/raycaster_internal.h"
#include "interrupts/keyboard.h"

#define GAME_SCANCODE_QUEUE_SIZE 64

// Raw keyboard scancodes are stored here
static volatile uint8_t g_game_scancode_queue[GAME_SCANCODE_QUEUE_SIZE];
// Head points to where next item is written
static volatile uint8_t g_game_scancode_head = 0;
// Tail points to where next item is read
static volatile uint8_t g_game_scancode_tail = 0;
// Bitmask with current held keys (WASDQE)
static volatile uint8_t g_game_key_state = 0;
// Set when user asked to start the game
static volatile uint8_t g_game_launch_requested = 0;
// Set when user asked to exit the game
static volatile uint8_t g_game_exit_requested = 0;
// Set when last scancode was 0xE0 prefix
static volatile uint8_t g_game_extended_scancode = 0;
// 1 when game should receive keyboard input
static volatile int g_game_input_active = 0;

// Add one scancode to the queue
static void raycaster_input_enqueue_scancode(uint8_t scancode)
{
    uint8_t next = (uint8_t)((g_game_scancode_head + 1) % GAME_SCANCODE_QUEUE_SIZE);
    if (next == g_game_scancode_tail) {
        // Queue full, drop the oldest item
        g_game_scancode_tail = (uint8_t)((g_game_scancode_tail + 1) % GAME_SCANCODE_QUEUE_SIZE);
    }
    g_game_scancode_queue[g_game_scancode_head] = scancode;
    g_game_scancode_head = next;
}

// Read one queued scancode if available
uint8_t raycaster_input_pop_scancode_internal(uint8_t *scancode)
{
    if (scancode == 0) {
        // Caller gave invalid output pointer
        return 0;
    }

    if (g_game_scancode_tail == g_game_scancode_head) {
        // Queue empty
        return 0;
    }

    *scancode = g_game_scancode_queue[g_game_scancode_tail];
    g_game_scancode_tail = (uint8_t)((g_game_scancode_tail + 1) % GAME_SCANCODE_QUEUE_SIZE);
    return 1;
}

// Enable or disable game input and reset state
void raycaster_input_set_active(int enabled)
{
    // Reset all transient input state when mode changes
    g_game_input_active = enabled;
    g_game_key_state = 0;
    g_game_exit_requested = 0;
    g_game_extended_scancode = 0;
    g_game_scancode_head = 0;
    g_game_scancode_tail = 0;
    (void)keyboard_get_last_key();
    if (enabled) {
        g_game_launch_requested = 0;
    }
}

// Return whether game input is active
int raycaster_input_is_active(void)
{
    return g_game_input_active;
}

// Ask the kernel loop to launch the game
void raycaster_input_request_launch(void)
{
    g_game_launch_requested = 1;
}

// Read and clear pending launch request
uint8_t raycaster_input_consume_launch_request(void)
{
    // Return current flag and clear it
    uint8_t requested = g_game_launch_requested;
    g_game_launch_requested = 0;
    return requested;
}

// Read and clear pending exit request
uint8_t raycaster_input_consume_exit_request_internal(void)
{
    // Return current flag and clear it
    uint8_t requested = g_game_exit_requested;
    g_game_exit_requested = 0;
    return requested;
}

// Ask the game loop to exit
void raycaster_input_request_exit(void)
{
    g_game_exit_requested = 1;
}

// Handle one incoming scancode and update key state
void raycaster_input_submit_scancode(uint8_t scancode)
{
    if (!g_game_input_active) {
        // Ignore keys while game input is disabled
        return;
    }

    // ESC make or break code should always request exit
    if ((scancode & 0x7F) == 0x01) {
        g_game_exit_requested = 1;
    }

    raycaster_input_enqueue_scancode(scancode);

    if (scancode == 0xE0) {
        // Next scancode is an extended key (arrow keys etc)
        g_game_extended_scancode = 1;
        return;
    }

    if (g_game_extended_scancode) {
        // Decode extended key and clear prefix state
        uint8_t released = (uint8_t)(scancode & 0x80);
        uint8_t code = (uint8_t)(scancode & 0x7F);
        g_game_extended_scancode = 0;

        // Map arrows to movement/turn keys
        if (code == 0x48) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_W;
            else g_game_key_state |= GAME_KEY_W;
        } else if (code == 0x50) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_S;
            else g_game_key_state |= GAME_KEY_S;
        } else if (code == 0x4B) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_Q;
            else g_game_key_state |= GAME_KEY_Q;
        } else if (code == 0x4D) {
            if (released) g_game_key_state &= (uint8_t)~GAME_KEY_E;
            else g_game_key_state |= GAME_KEY_E;
        }
        return;
    }

    if (scancode & 0x80) {
        // Break code means key released
        uint8_t released_key = (uint8_t)(scancode & ~0x80);
        if (released_key == 0x11) g_game_key_state &= (uint8_t)~GAME_KEY_W;
        else if (released_key == 0x1F) g_game_key_state &= (uint8_t)~GAME_KEY_S;
        else if (released_key == 0x1E) g_game_key_state &= (uint8_t)~GAME_KEY_A;
        else if (released_key == 0x20) g_game_key_state &= (uint8_t)~GAME_KEY_D;
        else if (released_key == 0x10) g_game_key_state &= (uint8_t)~GAME_KEY_Q;
        else if (released_key == 0x12) g_game_key_state &= (uint8_t)~GAME_KEY_E;
        return;
    }

    // Make code means key pressed
    if (scancode == 0x11) g_game_key_state |= GAME_KEY_W;
    else if (scancode == 0x1F) g_game_key_state |= GAME_KEY_S;
    else if (scancode == 0x1E) g_game_key_state |= GAME_KEY_A;
    else if (scancode == 0x20) g_game_key_state |= GAME_KEY_D;
    else if (scancode == 0x10) g_game_key_state |= GAME_KEY_Q;
    else if (scancode == 0x12) g_game_key_state |= GAME_KEY_E;
}

// Build the current control snapshot for this frame
char raycaster_poll_controls_internal(Raycaster *rc, uint8_t *key_down)
{
    uint8_t state = g_game_key_state;
    char instant_key = 0;

    // Continuous state from scancode tracking
    key_down[RC_KEY_W] = (state & GAME_KEY_W) ? 1 : 0;
    key_down[RC_KEY_S] = (state & GAME_KEY_S) ? 1 : 0;
    key_down[RC_KEY_A] = (state & GAME_KEY_A) ? 1 : 0;
    key_down[RC_KEY_D] = (state & GAME_KEY_D) ? 1 : 0;
    key_down[RC_KEY_Q] = (state & GAME_KEY_Q) ? 1 : 0;
    key_down[RC_KEY_E] = (state & GAME_KEY_E) ? 1 : 0;

    {
        // Instant key fallback from keyboard char buffer
        char key = keyboard_get_last_key();
        if (key == 27) {
            // ESC from character path also exits the game
            rc->game_running = false;
        } else if (key == 'w' || key == 'W' || key == 's' || key == 'S' ||
                   key == 'a' || key == 'A' || key == 'd' || key == 'D' ||
                   key == 'q' || key == 'Q' || key == 'e' || key == 'E') {
            char lower = key;
            if (lower >= 'A' && lower <= 'Z') {
                lower = (char)(lower - 'A' + 'a');
            }

            if (lower == 'w') key_down[RC_KEY_W] = 1;
            else if (lower == 's') key_down[RC_KEY_S] = 1;
            else if (lower == 'a') key_down[RC_KEY_A] = 1;
            else if (lower == 'd') key_down[RC_KEY_D] = 1;
            else if (lower == 'q') key_down[RC_KEY_Q] = 1;
            else if (lower == 'e') key_down[RC_KEY_E] = 1;

            instant_key = lower;
        }
    }

    return instant_key;
}
