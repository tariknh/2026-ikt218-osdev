#include "../../include/fedrelandet.h"
#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
#include "../../include/program.h"

// Prints all available programs and their corresponding index to select them.
void print_menu() {
  printf("Select one of the following programs:\n");
  for (int i = 0; i < PROGRAM_END; i++)
    printf("%d: %s\n", i, program_names[i]);
}

void print_piano_menu() {
  printf("Welcome to the PIANO\n");
  printf("To exit press 'q'\n");
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
  printf("Welcome to the RADIO\n");
  printf("To exit press 'q'\n");
  printf("press 0-5 to play a song :)\n");
}

// switches active program based on key presses
void menu_navigate(int key) {
  // Translate ASCII numbers to actual numbers.
  int key_char = key - 48;
  switch (key_char) {
  case PROGRAM_SHELL:
    active_program = PROGRAM_SHELL;
    printf("ACTIVE PROGRAM: SHELL\n");
    break;
  case PROGRAM_FEDRELANDET:
    active_program = PROGRAM_FEDRELANDET;
    printf("ACTIVE PROGRAM: FEDRELANDET\n");
    hedre_fedrelandet();

    break;
  case PROGRAM_PIANO:
    active_program = PROGRAM_PIANO;
    clearTerminal();
    printf("ACTIVE PROGRAM: PIANO\n");
    print_piano_menu();
    break;
  case PROGRAM_RADIO:
    active_program = PROGRAM_RADIO;
    print_radio_menu();
    printf("ACTIVE PROGRAM: RADIO\n");
    break;
  default:
    active_program = PROGRAM_MENU;
    clearTerminal();
    printf("ACTIVE PROGRAM: MENU\n");
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
