#include <drivers/input/keyboard.h>
#include <stdint.h>
#include <stdio.h>

int cmd_keyboard_logger(int argc, char** argv) {
  (void)argc;
  (void)argv;
  printf("Keyboard logger started. Type and press Enter; ESC or Ctrl+C exits.\n");

  while (1) {
    decode_keyboard();
    uint8_t key = 0;
    pop_key(&key);

    if (key != 0) {
      // ctrl -c or esc
      if (key == 27 || key == 3) {
        printf("\nLogger exited.\n");
        return 0;
      }
      printf("%c", key);
      fflush(stdout);
    }
  }
}
