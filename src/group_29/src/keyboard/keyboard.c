#include "keyboard.h"

#include "../printing/printing.h"

static scancode_buffer_t scancode_buffer;
static keycode_buffer_t keycode_buffer;
static ascii_buffer_t ascii_buffer;
static bool left_shift_pressed;
static bool right_shift_pressed;
static bool caps_lock_enabled;

void push_ascii_buffer(char value) {
    if (ascii_buffer.count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    ascii_buffer.buffer[ascii_buffer.head] = value;
    ascii_buffer.head = (uint16_t)((ascii_buffer.head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++ascii_buffer.count;
}

bool is_letter(char value) {
    return value >= 'a' && value <= 'z';
}

char uppercase(char value) {
    if (value < 'a' || value > 'z') {
        return value;
    }

    return (char)(value - ('a' - 'A'));
}

bool shift_active(void) {
    return left_shift_pressed || right_shift_pressed;
}

char translate_keycode_to_ascii(keycode_t keycode) {
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

void push_scancode_buffer(scancode_buffer_t* buffer, uint8_t scancode) {
    if (buffer->count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    buffer->buffer[buffer->head] = scancode;
    buffer->head = (uint16_t)((buffer->head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++buffer->count;
}

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

scancode_buffer_t create_scancode_buffer(void) {
    scancode_buffer_t buffer = {0};
    return buffer;
}

void push_keycode_buffer(keycode_buffer_t* buffer, keycode_t keycode) {
    if (buffer->count >= KEYBOARD_BUFFER_SIZE) {
        return;
    }

    buffer->buffer[buffer->head] = keycode;
    buffer->head = (uint16_t)((buffer->head + 1U) % KEYBOARD_BUFFER_SIZE);
    ++buffer->count;
}

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

keycode_buffer_t create_keycode_buffer(void) {
    keycode_buffer_t buffer = {0};
    return buffer;
}

keycode_t new_keycode(uint16_t keycode, bool release) {
    keycode_t value;
    value.keycode = keycode;
    value.release = release;
    return value;
}

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
    }
}

bool keyboard_has_char(void) {
    return ascii_buffer.count > 0U;
}

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
