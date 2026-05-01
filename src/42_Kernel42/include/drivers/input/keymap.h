#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
Scancode to ASCII map
*/
extern uint8_t* scancodeToAscii;
/**
 * Scancode to ASCII map while shift is being held
 */
extern uint8_t* scancodeToAsciiShift;

// NOTE: More mappings could be created, although more special characters where not needed for this
// project.

/**
 * Sets the current keymap
 * @param keymap NO or US
 * @return whether or not a valid keymap was requested and set
 */
bool set_keymap(const char* keymap);
