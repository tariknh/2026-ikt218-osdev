#include "../../include/fedrelandet.h"
#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
#include "../../include/piano.h"
#include "../../include/program.h"

// Prints all available programs and their corresponding index to select them.
void print_menu() {
  printf("Select one of the following programs:\n");
  printf("To exit any of the programs type 'ctrl-q':\n");
  for (int i = 0; i < PROGRAM_END; i++) {
    const char *program_name = program_names[i];
    if (i != PROGRAM_MENU) {
      printf("%d: %s\n", i, program_name);
    }
  }
}

void print_piano_menu() {
  printf("Program PIANO selected\n");
  printf("Matrix rain will start once you start playing\n");
  printf("To exit press 'ctrl-q'\n");
  printf("The following keys correspond to the following notes\n");
  printf("a = c\n");
  printf("s = d\n");
  printf("d = e\n");
  printf("f = f\n");
  printf("g = g\n");
  printf("h = a\n");
  printf("j = h\n");
  printf("k = c\n");
  printf("l = d\n");
}

void print_radio_menu() {
  printf("Program RADIO selected\n");
  printf("To exit press 'ctrl-q'\n");
  printf("press 0-5 to play a song :)\n");
}

void print_shell_menu() {
  printf("program SHELL selected\n");
  printf("To exit press 'ctrl-q'\n");
  printf("To write press any other character\n");
}
void print_fedrelandet_menu() { printf("Program FEDRELANDET selected\n"); }

// switches active program based on key presses
void menu_navigate(int key) {
  // Translate ASCII numbers to actual numbers.
  int key_char = key - 48;
  clearTerminal();
  switch (key_char) {
  case PROGRAM_SHELL:
    active_program = PROGRAM_SHELL;
    print_shell_menu();
    break;
  case PROGRAM_FEDRELANDET:
    active_program = PROGRAM_FEDRELANDET;
    hedre_fedrelandet();
    break;
  case PROGRAM_PIANO:
    piano_played_key = 0;
    active_program = PROGRAM_PIANO;
    print_piano_menu();
    break;
  case PROGRAM_RADIO:
    active_program = PROGRAM_RADIO;
    print_radio_menu();
    break;
  default:
    active_program = PROGRAM_MENU;
    print_menu();
    break;
  }
}
void menu_keyboard_handler(int scancode) {
  if (scancode & 0x80)
    return;
  int key = scancode_ascii[scancode];
  menu_navigate(key);
}
