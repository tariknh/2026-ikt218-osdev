#include "keyboard.h"

#include "../printing/printing.h"

enum {
    KEYCODE_BACKSPACE = 0x0E,
    KEYCODE_ENTER = 0x1C,
    KEYCODE_LEFT_SHIFT = 0x2A,
    KEYCODE_RIGHT_SHIFT = 0x36,
    KEYCODE_CAPS_LOCK = 0x3A,
};

typedef struct {
    char buffer[KEYBOARD_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} ascii_buffer_t;

static scancode_buffer_t scancode_buffer;
static keycode_buffer_t keycode_buffer;
static ascii_buffer_t ascii_buffer;
static bool left_shift_pressed;
static bool right_shift_pressed;
static bool caps_lock_enabled;

static const char keyboard_ascii_map[128] = {
    ['\x01'] = 0,
    ['\x02'] = '1', ['\x03'] = '2', ['\x04'] = '3', ['\x05'] = '4',
    ['\x06'] = '5', ['\x07'] = '6', ['\x08'] = '7', ['\x09'] = '8',
    ['\x0A'] = '9', ['\x0B'] = '0', ['\x0C'] = '-', ['\x0D'] = '=',
    ['\x0E'] = '\b', ['\x0F'] = '\t',
    ['\x10'] = 'q', ['\x11'] = 'w', ['\x12'] = 'e', ['\x13'] = 'r',
    ['\x14'] = 't', ['\x15'] = 'y', ['\x16'] = 'u', ['\x17'] = 'i',
    ['\x18'] = 'o', ['\x19'] = 'p', ['\x1A'] = '[', ['\x1B'] = ']',
    ['\x1C'] = '\n', ['\x1E'] = 'a', ['\x1F'] = 's', ['\x20'] = 'd',
    ['\x21'] = 'f', ['\x22'] = 'g', ['\x23'] = 'h', ['\x24'] = 'j',
    ['\x25'] = 'k', ['\x26'] = 'l', ['\x27'] = ';', ['\x28'] = '\'',
    ['\x29'] = '`', ['\x2B'] = '\\', ['\x2C'] = 'z', ['\x2D'] = 'x',
    ['\x2E'] = 'c', ['\x2F'] = 'v', ['\x30'] = 'b', ['\x31'] = 'n',
    ['\x32'] = 'm', ['\x33'] = ',', ['\x34'] = '.', ['\x35'] = '/',
    ['\x39'] = ' '
};

static const char keyboard_ascii_shift_map[128] = {
    ['\x01'] = 0,
    ['\x02'] = '!', ['\x03'] = '@', ['\x04'] = '#', ['\x05'] = '$',
    ['\x06'] = '%', ['\x07'] = '^', ['\x08'] = '&', ['\x09'] = '*',
    ['\x0A'] = '(', ['\x0B'] = ')', ['\x0C'] = '_', ['\x0D'] = '+',
    ['\x0E'] = '\b', ['\x0F'] = '\t',
    ['\x10'] = 'Q', ['\x11'] = 'W', ['\x12'] = 'E', ['\x13'] = 'R',
    ['\x14'] = 'T', ['\x15'] = 'Y', ['\x16'] = 'U', ['\x17'] = 'I',
    ['\x18'] = 'O', ['\x19'] = 'P', ['\x1A'] = '{', ['\x1B'] = '}',
    ['\x1C'] = '\n', ['\x1E'] = 'A', ['\x1F'] = 'S', ['\x20'] = 'D',
    ['\x21'] = 'F', ['\x22'] = 'G', ['\x23'] = 'H', ['\x24'] = 'J',
    ['\x25'] = 'K', ['\x26'] = 'L', ['\x27'] = ':', ['\x28'] = '"',
    ['\x29'] = '~', ['\x2B'] = '|', ['\x2C'] = 'Z', ['\x2D'] = 'X',
    ['\x2E'] = 'C', ['\x2F'] = 'V', ['\x30'] = 'B', ['\x31'] = 'N',
    ['\x32'] = 'M', ['\x33'] = '<', ['\x34'] = '>', ['\x35'] = '?',
    ['\x39'] = ' '
};

/// @brief Pushes one translated ASCII character into the ASCII buffer.
/// 
/// @param value The translated character to store for later shell input.
static void push_ascii_buffer(char value) {
    if (ascii_buffer.count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    ascii_buffer.buffer[ascii_buffer.head] = value;
    ascii_buffer.head = (uint16_t)((ascii_buffer.head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++ascii_buffer.count;
}

/// @brief Checks whether a translated ASCII character is a lowercase letter.
/// 
/// @param value The character to inspect.
/// @return True if the character is between `a` and `z`.
static bool is_letter(char value) {
    return value >= 'a' && value <= 'z';
}

/// @brief Converts one lowercase ASCII letter to uppercase.
/// 
/// @param value The character to convert.
/// @return The uppercase version of the character, or the original value if unchanged.
static char uppercase(char value) {
    if (value < 'a' || value > 'z') {
        return value;
    }

    return (char)(value - ('a' - 'A'));
}

/// @brief Checks whether either Shift key is currently pressed.
/// 
/// @return True if left or right Shift is active.
static bool shift_active(void) {
    return left_shift_pressed || right_shift_pressed;
}

/// @brief Translates one key event into an ASCII character.
/// 
/// @param keycode The decoded key event.
/// @return The translated ASCII character, or 0 if the key should not produce output.
static char translate_keycode_to_ascii(keycode_t keycode) {
    char ascii;

    if (keycode.release || keycode.keycode >= 128U) {
        return 0;
    }

    ascii = keyboard_ascii_map[keycode.keycode];
    if (ascii == 0) {
        return 0;
    }

    if (is_letter(ascii)) {
        if (shift_active() != caps_lock_enabled) {
            return uppercase(ascii);
        }

        return ascii;
    }

    if (shift_active()) {
        return keyboard_ascii_shift_map[keycode.keycode];
    }

    return ascii;
}

/// @brief Echoes one translated character to the terminal.
/// 
/// @param value The character to print to the screen.
static void print_keyboard_char(char value) {
    if (value == '\b') {
        if (main_interface.cursor.memory_position > main_interface.cursor.memory_start) {
            --main_interface.cursor.memory_position;
            *main_interface.cursor.memory_position = (uint16_t)(VgaColor(vga_black, vga_white) << 8 | ' ');
            main_interface.cursor.CalculateRowColFromMemoryPosition(&main_interface.cursor);
        }
        return;
    }

    {
        char output[2] = {value, 0};
        print_color(output, VgaColor(vga_black, vga_white));
    }
}

/// @brief Pushes a raw keyboard scancode into the scancode buffer.
/// 
/// @param buffer The scancode ring buffer to write to.
/// @param scancode The raw byte read from the keyboard controller.
void push_scancode_buffer(scancode_buffer_t* buffer, uint8_t scancode) {
    if (buffer->count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    buffer->buffer[buffer->head] = scancode;
    buffer->head = (uint16_t)((buffer->head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++buffer->count;
}

/// @brief Pops the oldest raw scancode from the scancode buffer.
/// 
/// @param buffer The scancode ring buffer to read from.
/// @return The oldest scancode in the buffer, or 0 if the buffer is empty.
uint8_t pop_scancode_buffer(scancode_buffer_t* buffer) {
    uint8_t value;

    if (buffer->count == 0U) {
        return 0;
    }

    value = buffer->buffer[buffer->tail];
    buffer->tail = (uint16_t)((buffer->tail + 1U) % KEYBOARD_BUFFER_SIZE);
    --buffer->count;
    return value;
}

/// @brief Creates and returns an empty scancode ring buffer.
/// 
/// @return A zero-initialized scancode buffer.
scancode_buffer_t create_scancode_buffer(void) {
    scancode_buffer_t buffer = {0};
    return buffer;
}

/// @brief Pushes a decoded key event into the keycode buffer.
/// 
/// @param buffer The keycode ring buffer to write to.
/// @param keycode The decoded key event to store.
void push_keycode_buffer(keycode_buffer_t* buffer, keycode_t keycode) {
    if (buffer->count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    buffer->buffer[buffer->head] = keycode;
    buffer->head = (uint16_t)((buffer->head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++buffer->count;
}

/// @brief Pops the oldest key event from the keycode buffer.
/// 
/// @param buffer The keycode ring buffer to read from.
/// @return The oldest key event in the buffer, or a zero-initialized keycode if empty.
keycode_t pop_keycode_buffer(keycode_buffer_t* buffer) {
    keycode_t value = {0};

    if (buffer->count == 0U) {
        return value;
    }

    value = buffer->buffer[buffer->tail];
    buffer->tail = (uint16_t)((buffer->tail + 1U) % KEYBOARD_BUFFER_SIZE);
    --buffer->count;
    return value;
}

/// @brief Creates and returns an empty keycode ring buffer.
/// 
/// @return A zero-initialized keycode buffer.
keycode_buffer_t create_keycode_buffer(void) {
    keycode_buffer_t buffer = {0};
    return buffer;
}

/// @brief Creates a keycode struct describing one key press or release event.
/// 
/// @param keycode The decoded keyboard key identifier.
/// @param release True if the key was released, false if it was pressed.
/// @return A populated keycode struct.
keycode_t new_keycode(uint16_t keycode, bool release) {
    keycode_t value;
    value.keycode = keycode;
    value.release = release;
    return value;
}

/// @brief Converts one buffered scancode into a keycode event.
/// 
/// @param input The input scancode buffer.
/// @param output The output keycode buffer.
void scancode2keycode(scancode_buffer_t* input, keycode_buffer_t* output) {
    uint8_t scancode;
    bool release;

    if (input->count == 0U) {
        return;
    }

    scancode = pop_scancode_buffer(input);
    if (scancode == 0xE0U || scancode == 0xE1U) {
        return;
    }

    release = (scancode & 0x80U) != 0U;
    push_keycode_buffer(output, new_keycode((uint16_t)(scancode & 0x7FU), release));
}

/// @brief Initializes keyboard buffers and modifier state.
void init_keyboard(void) {
    scancode_buffer = create_scancode_buffer();
    keycode_buffer = create_keycode_buffer();
    ascii_buffer.head = 0;
    ascii_buffer.tail = 0;
    ascii_buffer.count = 0;
    left_shift_pressed = false;
    right_shift_pressed = false;
    caps_lock_enabled = false;
}

/// @brief Processes one raw scancode from the keyboard controller.
/// 
/// @param scancode The raw byte read from port `0x60`.
void keyboard_handle_scancode(uint8_t scancode) {
    keycode_t keycode;
    char ascii;

    push_scancode_buffer(&scancode_buffer, scancode);
    scancode2keycode(&scancode_buffer, &keycode_buffer);

    while (keycode_buffer.count > 0U) {
        keycode = pop_keycode_buffer(&keycode_buffer);

        if (keycode.keycode == KEYCODE_LEFT_SHIFT) {
            left_shift_pressed = !keycode.release;
            continue;
        }

        if (keycode.keycode == KEYCODE_RIGHT_SHIFT) {
            right_shift_pressed = !keycode.release;
            continue;
        }

        if (keycode.keycode == KEYCODE_CAPS_LOCK && !keycode.release) {
            caps_lock_enabled = !caps_lock_enabled;
            continue;
        }

        ascii = translate_keycode_to_ascii(keycode);
        if (ascii == 0) {
            continue;
        }

        push_ascii_buffer(ascii);
        print_keyboard_char(ascii);
    }
}

/// @brief Checks whether a translated ASCII character is available.
/// 
/// @return True if the ASCII buffer contains at least one character.
bool keyboard_has_char(void) {
    return ascii_buffer.count > 0U;
}

/// @brief Pops the oldest translated ASCII character.
/// 
/// @return The oldest character in the ASCII buffer, or 0 if the buffer is empty.
char keyboard_pop_char(void) {
    char value;

    if (ascii_buffer.count == 0U) {
        return 0;
    }

    value = ascii_buffer.buffer[ascii_buffer.tail];
    ascii_buffer.tail = (uint16_t)((ascii_buffer.tail + 1U) % KEYBOARD_BUFFER_SIZE);
    --ascii_buffer.count;
    return value;
}
