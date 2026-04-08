#include "terminal.h"
#include "string.h"
#include "shell.h"
#include "memory.h"

//possible to add more commands for help here
void command_help(){
    terminal_write("\nCommands avalible:\nclear\nhello\nhelp\n");
}

void command_hello() {
    terminal_write("\nHello, this is an orange shell terminal made by Oscar and Herman\n");
}

void command_clear() {
    terminal_initialize();
    terminal_update_cursor();
}

void shell_execute_command(char* input) {
    if(strlen(input) == 0) {
        return;
    }
    else if(strcmp(input, "clear") == 0) {
        command_clear();
    }
    else if(strcmp(input, "hello") == 0) {
        command_hello();
    }
    else if(strcmp(input, "help") == 0) {
        command_help();
    }
    else {
        terminal_write("\n");
        terminal_write(input);
        terminal_write(" is not a command, write help for avalible commands\n");
    }
}