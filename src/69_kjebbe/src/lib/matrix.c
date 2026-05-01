#include "../../include/kernel/pit.h"
#include "../../include/libc/stdio.h"

char *width_string = "0000000000000000000000000000000000000000000000000000000"
                     "0000000000000000000000000";

char *width1_string = "00000000000000000000";

char *width2_string = "00000";

char *width3_string = "0000000000";

char *width4_string = "00000";

char *width5_string = "0000000000000000000000000000000000000000";

// Defines top and bottom strips for the Norwegian flag
void print_strip() {
  printf_color(width1_string, VGA_RED);
  printf_color(width2_string, VGA_WHITE);
  printf_color(width3_string, VGA_BLUE);
  printf_color(width4_string, VGA_WHITE);
  printf_color(width5_string, VGA_RED);
}

// Prints top part of Norwegian flag
void print_top() {
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
}

// Prints bottom part of Norwegian flag
void print_bottom() {
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
}

void print_middle_strip() {
  printf_color(width1_string, VGA_WHITE);
  printf_color(width2_string, VGA_WHITE);
  printf_color(width3_string, VGA_BLUE);
  printf_color(width4_string, VGA_WHITE);
  printf_color(width5_string, VGA_WHITE);
}
// Prints middle part of Norwegian flag
void print_middle() {
  print_middle_strip();
  print_middle_strip();
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  print_middle_strip();
  print_middle_strip();
}

// Main function to print Norwegian flag
void ja_vi_elsker() {
  clearTerminal();
  print_top();
  print_middle();
  print_bottom();
}

// pseudo random number generator
static uint32_t seed = 12345;
static uint32_t rng() {
  seed = seed * 1664525 + 1013904223;
  return seed;
}

int matrix_current_small = 'a';
int matrix_current_large = 'A';
int matrix_current_number = '0';

// Changes the characters to be printed based on piano offset
void update_matrix_current(int offset) {
  switch (offset % 12) {
  case (0):
    matrix_current_small = 'c';
    matrix_current_large = 'C';
    matrix_current_number = '0';
    break;
  case (1):
    matrix_current_small = 'c';
    matrix_current_large = 'C';
    matrix_current_number = '1';
    break;
  case (2):
    matrix_current_small = 'd';
    matrix_current_large = 'D';
    matrix_current_number = '2';
    break;
  case (3):
    matrix_current_small = 'd';
    matrix_current_large = 'D';
    matrix_current_number = '3';
    break;

  case (4):
    matrix_current_small = 'e';
    matrix_current_large = 'E';
    matrix_current_number = '4';
    break;

  case (5):
    matrix_current_small = 'f';
    matrix_current_large = 'F';
    matrix_current_number = '5';
    break;
  case (6):
    matrix_current_small = 'f';
    matrix_current_large = 'F';
    matrix_current_number = '6';
    break;
  case (7):
    matrix_current_small = 'g';
    matrix_current_large = 'G';
    matrix_current_number = '7';
    break;
  case (8):
    matrix_current_small = 'g';
    matrix_current_large = 'G';
    matrix_current_number = '8';
    break;
  case (9):
    matrix_current_small = 'a';
    matrix_current_large = 'A';
    matrix_current_number = '9';
    break;
  case (10):
    matrix_current_small = 'a';
    matrix_current_large = 'A';
    matrix_current_number = '0';
    break;
  case (11):
    matrix_current_small = 'b';
    matrix_current_large = 'B';
    matrix_current_number = '1';
    break;
  }
}

// Reload values for each character to be printed based on what the piano plays.
const int reload_small = 30;
int counter_small = reload_small;
const int reload_large = 25;
int counter_large = reload_large;
const int reload_number = 48;
int counter_number = reload_number;

// Renders a single frame of the matrix rain
void matrix_rain_frame() {
  static int drop[80] = {0};

  for (int x = 0; x < 80; x++) {
    int y = drop[x];

    // Piano coupled animation
    if (--counter_small == 0) {
      terminal_set_char(rng() % 80, rng() % 25, matrix_current_small,
                        VGA_RED_ON_BLACK);
      // Clear random index on screen.
      terminal_set_char(rng() % 80, rng() % 25, ' ', VGA_WHITE_ON_BLACK);
      counter_small = reload_small;
    }
    if (--counter_large == 0) {
      terminal_set_char(rng() % 80, rng() % 25, matrix_current_large,
                        VGA_BLUE_ON_BLACK);
      // Clear random index on screen.
      terminal_set_char(rng() % 80, rng() % 25, ' ', VGA_WHITE_ON_BLACK);
      counter_large = reload_large;
    }
    if (--counter_number == 0) {
      terminal_set_char((rng() * rng()) % 80, rng() % 25, matrix_current_number,
                        VGA_WHITE_ON_BLACK);
      // Clear random index on screen.
      terminal_set_char(rng() % 80, rng() % 25, ' ', VGA_WHITE_ON_BLACK);
      counter_number = reload_number;
    }

    // Matrix rainfall
    // Set lowest row to yellow colour
    terminal_set_char(x, y, '0' + (rng() % 10), VGA_YELLOW_ON_BLACK);
    // Set trail to green colour
    terminal_set_char(x, (y + 24) % 25, '0' + (rng() % 10),
                      VGA_LIGHT_GREEN_ON_BLACK);
    // Clear previous row
    terminal_set_char(x, (y + 23) % 25, ' ', VGA_WHITE_ON_BLACK);

    // 1/10 chance for raindrop to fall one step lower.
    if (rng() % 10 == 1) {
      drop[x] = (drop[x] + 1) % 25;
    }
  }
}
