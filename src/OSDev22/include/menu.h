#ifndef MENU_H
#define MENU_H

#include "libc/stdint.h"
#include "libc/stdbool.h"


typedef enum {
    MENU_OPT_PONG = 0,
    MENU_OPT_PIANO,
    MENU_OPT_SNAKE,
    MENU_OPT_NOTES,
    MENU_OPT_ABOUT,
    MENU_OPT_EXIT,
    MENU_OPT_COUNT
} MenuOption;

void run_menu(void);

void show_about(void);

#endif /* MENU_H */