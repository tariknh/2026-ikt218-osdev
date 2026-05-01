#include "stdint.h"
#include "string.h"
#include "drivers/input/keymap.h"

extern uint8_t scancodeToAsciiShift_no[255];
extern uint8_t scancodeToAscii_no[255];
extern uint8_t scancodeToAsciiShift_us[255];
extern uint8_t scancodeToAscii_us[255];

uint8_t* scancodeToAsciiShift;
uint8_t* scancodeToAscii;

bool set_keymap(const char *keymap) {
    if (strcmp(keymap, "us") == 0) {
        scancodeToAscii = scancodeToAscii_us;
        scancodeToAsciiShift = scancodeToAsciiShift_us;
        return true;
    }
    if (strcmp(keymap, "no") == 0) {
        scancodeToAscii = scancodeToAscii_no;
        scancodeToAsciiShift = scancodeToAsciiShift_no;
        return true;
    }
    return false;
}