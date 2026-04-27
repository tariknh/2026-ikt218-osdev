#include "shell/shell_command.h"

#include <stdio.h>
#include <string.h>

#define MAX_ARGS 16
#define MAX_ARG_LEN 64

extern const shell_command_t command_table[];
extern const size_t NUM_COMMANDS;

static char argv_static[MAX_ARGS][MAX_ARG_LEN];
static char* argv_ptrs[MAX_ARGS];

static int parse_args_fixed(const char* line, int* argc_out) {
  *argc_out = 0;
  if (!line || !*line)
    return -1;

  const char* p = line;
  int argi = 0;
  int arglen = 0;

  while (*p && argi < MAX_ARGS) {
    while (*p == ' ' || *p == '\t')
      p++;
    if (!*p)
      break;

    char* dest = argv_static[argi];
    argv_ptrs[argi] = dest;
    while (*p && *p != ' ' && *p != '\t' && arglen < MAX_ARG_LEN - 1) {
      *dest++ = *p++;
      arglen++;
    }
    *dest = '\0';
    arglen = 0;

    argi++;
    if (*p)
      p++;
  }

  *argc_out = argi;
  return (argi == 0 || argi >= MAX_ARGS) ? -1 : 0;
}

int execute_command(const char* line) {
  int argc = 0;
  if (parse_args_fixed(line, &argc) != 0) {
    printf("Parse error (max %d args)\n", MAX_ARGS);
    return -1;
  }

  if (argc == 0)
    return -1;

  const char* cmd_name = argv_ptrs[0];
  for (size_t i = 0; i < NUM_COMMANDS; i++) {
    if (strcmp(cmd_name, command_table[i].name) == 0) {
      return command_table[i].execute(argc, argv_ptrs);
    }
  }

  printf("Unknown command: %s\n", cmd_name);
  return -1;
}
