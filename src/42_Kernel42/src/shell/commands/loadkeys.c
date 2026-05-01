#include "drivers/input/keymap.h"
#include "stdio.h"

int cmd_loadkeys(int argc, char** argv) {
  if (argc > 1) {
    if (set_keymap(argv[1])) {
      // success
      printf("Loaded keymap (%s)\n", argv[1]);
      return 1;
    }
    // error
    printf("No keymap matching: \"%s\"\n\n", argv[1]);
  }

  printf("Usage: loadkeys <locale>\n\n");
  printf("Available keymaps:\n");
  printf("\t<us>: American US qwerty\n");
  printf("\t<no>: Norwegian qwerty\n");

  return 0;
}
