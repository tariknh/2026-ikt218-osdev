#include <stdio.h>

int cmd_echo(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    printf("%s%c", argv[i], (i + 1 < argc) ? ' ' : '\n');
  }
  if (argc <= 1)
    printf("\n");
  return 0;
}
