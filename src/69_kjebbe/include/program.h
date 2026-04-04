#ifndef PROGRAM_H
#define PROGRAM_H

// Defines enums for all existing programs on KjebbeOS
typedef enum {
  PROGRAM_SHELL = 0,
  PROGRAM_PIANO,
  PROGRAM_RADIO,
  PROGRAM_MENU,
  PROGRAM_FEDRELANDET,
  // this must always be at the bottom
  PROGRAM_END,
} program_t;

static const char *program_names[] = {
    [PROGRAM_SHELL] = "shell",
    [PROGRAM_PIANO] = "piano",
    [PROGRAM_RADIO] = "radio",
    [PROGRAM_MENU] = "menu",
    [PROGRAM_FEDRELANDET] = "FEDRELANDET",
};

extern program_t active_program;

#endif
