#include "../include/keyboard.h" // You'll need this header
#include "../include/io.h"       // For inb()
#include "../include/monitor.h"


#define BUFFER_SIZE 1024
char keyboard_buffer[BUFFER_SIZE];
int buffer_index = 0;
static key_action_t pending_action = KEY_ACTION_NONE;
static int pending_char = -1;
static keyboard_mode_t keyboard_mode = KEYBOARD_MODE_COMMAND;
static int text_start_row = 2;
static int text_start_column = 5;
static int text_width = 75;
static int text_height = 23;
static int extended_scancode = 0;
static int preferred_column = 5;


unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b', /* 14 */
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* 28 */
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, /* 42 */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    /* The ' ' above is at index 57! */
};

int cursor_pos = (2 * 160) + (5 * 2);   //padding of 5 spaces, starting at row 2 (0-indexed)
int shift_pressed = 0;

static int editor_left_pos(void) {
    return (text_start_row * 160) + (text_start_column * 2);
}

static int editor_right_column(void) {
    return text_start_column + text_width - 1;
}

static int editor_bottom_row(void) {
    return text_start_row + text_height - 1;
}

static int editor_row_from_pos(int position) {
    return position / 160;
}

static int editor_column_from_pos(int position) {
    return (position % 160) / 2;
}

static int editor_pos(int row, int column) {
    return (row * 160) + (column * 2);
}

static void editor_update_preferred_column(void) {
    preferred_column = editor_column_from_pos(cursor_pos);
}

static void editor_clear_line(char* video_memory, int row) {
    int column;

    for (column = text_start_column; column <= editor_right_column(); column++) {
        int pos = editor_pos(row, column);
        video_memory[pos] = ' ';
        video_memory[pos + 1] = 0x07;
    }
}

static void editor_scroll_up(char* video_memory) {
    int row;
    int column;

    for (row = text_start_row; row < editor_bottom_row(); row++) {
        for (column = text_start_column; column <= editor_right_column(); column++) {
            int dest = editor_pos(row, column);
            int src = editor_pos(row + 1, column);
            video_memory[dest] = video_memory[src];
            video_memory[dest + 1] = video_memory[src + 1];
        }
    }

    editor_clear_line(video_memory, editor_bottom_row());
}

static int editor_line_end(char* video_memory, int row) {
    int column;

    for (column = editor_right_column(); column >= text_start_column; column--) {
        int pos = editor_pos(row, column);
        if (video_memory[pos] != ' ' && video_memory[pos] != 0) {
            return pos + 2;
        }
    }

    return editor_pos(row, text_start_column);
}

static int editor_clamp_column_for_row(char* video_memory, int row, int desired_column) {
    int line_end = editor_line_end(video_memory, row);
    int max_column = editor_column_from_pos(line_end);

    if (desired_column < text_start_column) {
        desired_column = text_start_column;
    }

    if (desired_column > max_column) {
        desired_column = max_column;
    }

    return desired_column;
}

static void editor_advance_to_next_line(char* video_memory) {
    int next_row = editor_row_from_pos(cursor_pos) + 1;

    if (next_row > editor_bottom_row()) {
        editor_scroll_up(video_memory);
        next_row = editor_bottom_row();
    }

    cursor_pos = editor_pos(next_row, text_start_column);
    editor_update_preferred_column();
}

static void editor_move_left(char* video_memory) {
    if (cursor_pos <= editor_left_pos()) {
        return;
    }

    if (editor_column_from_pos(cursor_pos) == text_start_column) {
        int prev_row = editor_row_from_pos(cursor_pos) - 1;
        cursor_pos = editor_line_end(video_memory, prev_row);
    } else {
        cursor_pos -= 2;
    }

    editor_update_preferred_column();
}

static void editor_move_right(char* video_memory) {
    int row = editor_row_from_pos(cursor_pos);
    int column = editor_column_from_pos(cursor_pos);

    if (column < editor_right_column()) {
        cursor_pos += 2;
        return;
    }

    if (row < editor_bottom_row()) {
        cursor_pos = editor_pos(row + 1, text_start_column);
    }

    editor_update_preferred_column();
}

static void editor_move_up(char* video_memory) {
    int row = editor_row_from_pos(cursor_pos);

    if (row > text_start_row) {
        int target_column = editor_clamp_column_for_row(video_memory, row - 1, preferred_column);
        cursor_pos = editor_pos(row - 1, target_column);
    }
}

static void editor_move_down(char* video_memory) {
    int row = editor_row_from_pos(cursor_pos);

    if (row < editor_bottom_row()) {
        int target_column = editor_clamp_column_for_row(video_memory, row + 1, preferred_column);
        cursor_pos = editor_pos(row + 1, target_column);
    }
}


// 2. Define the cursor helper BEFORE the handler uses it
void update_hardware_cursor(int position) {
    uint16_t vga_offset = position / 2;

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((vga_offset >> 8) & 0xFF));

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(vga_offset & 0xFF));
}



key_action_t keyboard_take_action(void) {
    key_action_t action = pending_action;
    pending_action = KEY_ACTION_NONE;
    return action;
}

int keyboard_take_char(void) {
    int value = pending_char;
    pending_char = -1;
    return value;
}

void keyboard_set_mode(keyboard_mode_t mode) {
    keyboard_mode = mode;
}

void keyboard_enter_text_mode(int row, int column, int width, int height) {
    keyboard_mode = KEYBOARD_MODE_TEXT_EDITOR;
    text_start_row = row;
    text_start_column = column;
    text_width = width;
    text_height = height;
    cursor_pos = (row * 160) + (column * 2);
    preferred_column = column;
    buffer_index = 0;
    keyboard_buffer[0] = '\0';
    update_hardware_cursor(cursor_pos);
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    char* video_memory = (char*)0xb8000;
    int is_extended;

    if (scancode == 0xE0) {
        extended_scancode = 1;
        return;
    }

    is_extended = extended_scancode;
    extended_scancode = 0;

    // --- 1. Check for Shift Press/Release ---
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; return; }

    if (scancode == 0x01) { pending_action = KEY_ACTION_ESCAPE; return; }

    if (keyboard_mode == KEYBOARD_MODE_COMMAND) {
        if (scancode == 0x1C) { pending_action = KEY_ACTION_ENTER; return; }
    }

    // --- 1.5 Scrollback shortcuts ---
    if (scancode == 0x3B) { monitor_scroll_up(); return; }   // F1
    if (scancode == 0x3C) { monitor_scroll_down(); return; } // F2

    if (keyboard_mode != KEYBOARD_MODE_TEXT_EDITOR) {
        if (!(scancode & 0x80)) {
            char ascii_char = kbd_us[scancode];
            if (ascii_char != 0) {
                if (shift_pressed && ascii_char >= 'a' && ascii_char <= 'z') {
                    ascii_char -= 32;
                }
                pending_char = (int)ascii_char;
            }
        }
        return;
    }

    if (is_extended) {
        if (scancode == 0x4B) {
            editor_move_left(video_memory);
        } else if (scancode == 0x4D) {
            editor_move_right(video_memory);
        } else if (scancode == 0x48) {
            editor_move_up(video_memory);
        } else if (scancode == 0x50) {
            editor_move_down(video_memory);
        }
        update_hardware_cursor(cursor_pos);
        return;
    }

    // --- 2. Handle Backspace ---
    if (scancode == 0x0E) {
        int text_start_pos = editor_left_pos();

        // A. Prevent deleting the very first position
        if (cursor_pos <= text_start_pos) {
            return;
        }

        // B. If we are at the left margin, jump to the previous text row
        if ((cursor_pos % 160) == (text_start_column * 2)) {
            int prev_row = (cursor_pos / 160) - 1;
            int search_pos = editor_pos(prev_row, editor_right_column());
            int found = 0;

            while (search_pos >= (prev_row * 160) + (text_start_column * 2)) {
                if (video_memory[search_pos] != ' ' && video_memory[search_pos] != 0) {
                    cursor_pos = search_pos;
                    found = 1;
                    break;
                }
                search_pos -= 2;
            }

            if (!found) {
                cursor_pos = editor_pos(prev_row, text_start_column);
            }
        } else {
            cursor_pos -= 2;
        }

        if (cursor_pos < text_start_pos) {
            cursor_pos = text_start_pos;
        }

        editor_update_preferred_column();

        video_memory[cursor_pos] = ' ';
        video_memory[cursor_pos + 1] = 0x07;

        {
            int row = editor_row_from_pos(cursor_pos);
            int column;

            for (column = editor_column_from_pos(cursor_pos) + 1; column <= editor_right_column(); column++) {
                int dest = editor_pos(row, column - 1);
                int src = editor_pos(row, column);
                video_memory[dest] = video_memory[src];
                video_memory[dest + 1] = video_memory[src + 1];
            }

            {
                int last = editor_pos(row, editor_right_column());
                video_memory[last] = ' ';
                video_memory[last + 1] = 0x07;
            }
        }

        // Keep the buffer in sync
        if (buffer_index > 0) {
            buffer_index--;
            keyboard_buffer[buffer_index] = '\0';
        }

        update_hardware_cursor(cursor_pos);
        return;
    }

    // --- 3. Handle Enter ---
    if (scancode == 0x1C) {
        editor_advance_to_next_line(video_memory);
    } 
    else if (!(scancode & 0x80)) { 
        // --- 4. Regular Character Translation ---
        char ascii_char = kbd_us[scancode];
        if (ascii_char != 0) {
            if (shift_pressed && ascii_char >= 'a' && ascii_char <= 'z') ascii_char -= 32;

            // Store in Buffer
            if (buffer_index < BUFFER_SIZE - 1) {
                keyboard_buffer[buffer_index++] = ascii_char;
                keyboard_buffer[buffer_index] = '\0';
            }

            // Print
            video_memory[cursor_pos] = ascii_char;
            video_memory[cursor_pos + 1] = 0x07;
            cursor_pos += 2;
            editor_update_preferred_column();

            if (editor_column_from_pos(cursor_pos) > editor_right_column()) {
                editor_advance_to_next_line(video_memory);
            }
        }
    }

    // --- 5. Global Safety & Hardware Sync ---
    if (cursor_pos >= 160 * 25) {
        cursor_pos = editor_pos(editor_bottom_row(), text_start_column);
    }
    update_hardware_cursor(cursor_pos);
}
