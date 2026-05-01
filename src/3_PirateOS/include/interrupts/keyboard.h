#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void init_keyboard();             // Initialize the keyboard by registering our keyboard callback function
    void keyboard_print_prompt(void); // This function can be called to print a new prompt on the terminal and ensure the user is ready to type a new command. It also scrolls to the bottom of the terminal to ensure the prompt is visible and the user can see their input as they type.
    char keyboard_get_last_key(void); // // This function can be called by games to get the last key that was pressed by the user. It returns the ASCII character of the last key pressed, or 0 if no key has been pressed since the last time this function was called. After returning the last key, it clears the stored value to ensure that subsequent calls will only return a key if a new one has been pressed.

#ifdef __cplusplus
}
#endif

#endif
