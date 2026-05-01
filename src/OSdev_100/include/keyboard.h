#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum {
    KEY_ACTION_NONE = 0,
    KEY_ACTION_ENTER,
    KEY_ACTION_ESCAPE
} key_action_t;

typedef enum {
    KEYBOARD_MODE_COMMAND = 0,
    KEYBOARD_MODE_TEXT_EDITOR
} keyboard_mode_t;

// This tells other files that this function exists
void keyboard_handler(void);
key_action_t keyboard_take_action(void);
int keyboard_take_char(void);
void keyboard_set_mode(keyboard_mode_t mode);
void keyboard_enter_text_mode(int row, int column, int width, int height);

#endif
