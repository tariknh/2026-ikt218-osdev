#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_ARROW_UP 1 // Special key code for arrow up

#define KEY_ARROW_DOWN 2 // Special key code for arrow down

void keyboard_install(void); // Initializes the keyboard driver
char keyboard_get_char(void); // Gets one character from the keyboard buffer
char keyboard_read_char(void); // Waits for and reads one character
int keyboard_has_key(void); // Checks if a key is available


#endif