#ifndef PROGRAM_H
#define PROGRAM_H

// Defines enums for all existing programs on KjebbeOS
typedef enum {
  PROGRAM_SHELL = 0,
  PROGRAM_PIANO,
  PROGRAM_RADIO,
  PROGRAM_FEDRELANDET,
  PROGRAM_MENU,
  // this must always be at the bottom
  PROGRAM_END,
} program_t;

static const char *program_names[] = {
    [PROGRAM_SHELL] = "shell", [PROGRAM_PIANO] = "piano",
    [PROGRAM_RADIO] = "radio", [PROGRAM_FEDRELANDET] = "FEDRELANDET",
    [PROGRAM_MENU] = "menu",
};

extern program_t active_program;

#endif
