#pragma once

#include "../string/string.h"
#include "../printing/printing.h"
#include "../libc/stdint.h"
#include "../user/user.h"
#include "../disk/ata.h"
#include "../fs/tinyfs.h"

#define MAX_ARGS 10

// Data structures

enum {
    COMMAND_VALID_ARGUMENTS = 3,
    COMMAND_SHELL_CLEARED = 2,
    COMMAND_SUCCESS = 0,
    COMMAND_STATUS_INVALID = -1,
    COMMAND_STATUS_UNKNOWN = -2,
    COMMAND_UNKNOWN_ARGUMENT = -3,
    COMMAND_TOO_FEW_ARGUMENTS = -4,
    COMMAND_TOO_MANY_ARGUMENTS = -5,
    COMMAND_ARGUMENT_INVALID = -6,
    COMMAND_ARGUMENT_USERNAME_TOO_LONG = -7,
    COMMAND_ARGUMENT_INVALID_SONG_NUMBER = -8
};

/// @brief Function pointer type for commands
typedef int8_t (*CommandFunction)(int argument_count, char* arguments[]);

/// @brief Describes a shell command
typedef struct {
    const char* name;
    const char* options;
    const char* description;
    CommandFunction command_function;
} CommandEntry;


typedef struct {
    uint8_t name;
    uint8_t options;
    uint8_t description;
} CommandColors;

typedef struct {
    char* song_name;
} SongMapEntry;

// Commands

/// @brief Displays all commands
int8_t command_help(int argument_count, char *arguments[]);

/// @brief Print user input to screen
int8_t command_echo(int argument_count, char* arguments[]);

/// @brief Clears the shell
int8_t command_clear(int argument_count, char *arguments[]);

/// @brief Sets the username of user
int8_t command_set_username(int argument_count, char* arguments[]);

/// @brief Displays memory info
int8_t command_heapinfo(int argument_count, char* arguments[]);

/// @brief Plays a song
int8_t command_playsong(int argument_count, char* arguments[]);

int8_t command_disktest(int argument_count, char* arguments[]);
int8_t command_format(int argument_count, char* arguments[]);
int8_t command_ls(int argument_count, char* arguments[]);
int8_t command_write_file(int argument_count, char* arguments[]);
int8_t command_cat(int argument_count, char* arguments[]);


// Command parsing/handling

/// @brief Parses input into arguments (argument_count/argument_count style)
int8_t parse_command_aguments(char* input, char* arguments[], int max_args);

/// @brief Dispatches command to the correct handler
int8_t handle_command(int argument_count, char* arguments[]);

/// @brief Parses and runs a command
///
/// @param input Non-const input string (it is changed while parsing)
///
/// @returns Result of the command, or negative on failure
int8_t run_command(char* input);
