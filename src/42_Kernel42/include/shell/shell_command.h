#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int (*command_handler_t)(int argc, char** argv);

typedef struct {
  const char* name;
  const char* description;
  command_handler_t execute;
} shell_command_t;

extern const shell_command_t command_table[];
extern const size_t NUM_COMMANDS;

/**
 * Executes a shell command from a provided line of text
 * @param line of text
 * @return -1 if failed to parse or too few arguments have bene provided
 */
int execute_command(const char* line);
