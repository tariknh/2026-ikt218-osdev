#include "terminal.h"
#include "string.h"
#include "shell.h"
#include "memory.h"
#include "song/song.h"
#include "pit.h"
#include "vga_mode13.h"
#include "mouse.h"

int frame = 0;
static uint32_t parse_uint(const char* str) {
    uint32_t result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}
Song Song_1 = {
    .notes = music_1,
    .length = sizeof(music_1) / sizeof(Note)
};
Song Song_2 = {
    .notes = music_2,
    .length = sizeof(music_2) / sizeof(Note)
};
Song Song_3 = {
    .notes = music_3,
    .length = sizeof(music_3) / sizeof(Note)
};
Song Song_4 = {
    .notes = music_4,
    .length = sizeof(music_4) / sizeof(Note)
};
Song Song_5 = {
    .notes = music_5,
    .length = sizeof(music_5) / sizeof(Note)
};
Song Song_6 = {
    .notes = music_6,
    .length = sizeof(music_6) / sizeof(Note)
};
Song starwars = {
    .notes = starwars_theme,
    .length = sizeof(starwars_theme) / sizeof(Note)
};
Song battlefield_1942 = {
    .notes = battlefield_1942_theme,
    .length = sizeof(battlefield_1942_theme) / sizeof(Note)
};


//possible to add more commands for help here
void command_help(){
    terminal_write("\nCommands avalible:\nclear\nhello\nmemory\ntriangle\nsleep_b <value>\nsleep_i <value>\nsong <song number>\ndraw (right click to leave)\nhelp\n");
}

void command_triangle() {
    terminal_write("\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEWEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEv iIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAWv   rEAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAA6v      RAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAE0        v1AAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAm           0AAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAIv             iAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAARv               vIAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAWv                 vWAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAA0                     WAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAEr                       1AAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAANm                         iAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAANr                           rIAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
}
void command_hello() {
    terminal_write("\nHello, this is an orange shell terminal made by Herman and Oscar");
}

void command_clear() {
    terminal_initialize();
    terminal_update_cursor();
}
void command_memory(uint32_t mb2_info) {
    print_heap_info();
    print_memory_layout(mb2_info);
}
void command_sleep_busy(const char* args) {
    __asm__("sti");
    uint32_t seconds = parse_uint(args);
    terminal_write("\n");
    if (seconds == 0) {
        terminal_write("\nUsage: sleep_busy <seconds>\n");
        return;
    }

    for (uint32_t i = 1; i <= seconds; i++) {
        terminal_write_dec(i);
        terminal_write("\n");
        sleep_busy(1000);
    }
    terminal_write("Done.\n");
}

void command_sleep_interrupt(const char* args) {
    uint32_t seconds = parse_uint(args);
    terminal_write("\n");
    if (seconds == 0) {
        terminal_write("\nUsage: sleep_interrupt <seconds>\n");
        return;
    }

    for (uint32_t i = 1; i <= seconds; i++) {
        terminal_write_dec(i);
        terminal_write("\n");
        sleep_interrupt(1000);
    }
    terminal_write("Done.\n");
}
void command_song(const char* args) {
    uint32_t song_number = parse_uint(args);
    switch(song_number) {
        case 1:
            terminal_write("\nNow playing mario\n");
            play_song(&Song_1);
            break;
        case 2:
            terminal_write("\now playing music_2\n");
            play_song(&Song_2);
            break;
        case 3:
            terminal_write("\now playing music_3\n");
            play_song(&Song_3);
            break;
        case 4:
            terminal_write("\nNow playing music_4\n");
            play_song(&Song_4);
            break;
        case 5:
            terminal_write("\nNow playing music_5\n");
            play_song(&Song_5);
            break;
        case 6:
            terminal_write("\nNow playing music_6\n");
            play_song(&Song_6);
            break;
        case 7:
            terminal_write("\nNow playing starwars theme\n");
            play_song(&starwars);
            break;
        case 8:
            terminal_write("\nNow playing Battlefield 1942 theme\n");
            play_song(&battlefield_1942);
            break;
        default:
            terminal_write("\nInvalid song number. Please choose a valid song (1-8).\n");
    }
}
void command_draw() {
    terminal_save_screen();
    vga_enter_mode13();
    vga_clear(0); 
    mouse_set_drawing_mode(1);  
    while (!mouse_right_clicked()) {
        __asm__("hlt");
    }
    mouse_set_drawing_mode(0);
    vga_exit_mode13();
    terminal_restore_screen();
    terminal_write("\n");
}

void shell_execute_command(char* input) {
    if(strlen(input) == 0) {
        return;
    }

    char* cmd = input;
    char* args = "";
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ' ') {
            input[i] = '\0';      // null-terminate the command part
            args = &input[i + 1]; // args starts after the space
            break;
        }
    }

    if(strcmp(input, "clear") == 0) {
        command_clear();
    }
    else if(strcmp(input, "hello") == 0) {
        command_hello();
    }
    else if(strcmp(input, "help") == 0) {
        command_help();
    }
    else if(strcmp(input, "memory") == 0) {
        command_memory(g_mb2_info);
    }
    else if(strcmp(input, "triangle") == 0) {
        command_triangle();
    }
    else if(strcmp(cmd, "sleep_b") == 0) {
        command_sleep_busy(args);
    }
    else if(strcmp(cmd, "sleep_i") == 0) {
        command_sleep_interrupt(args);
    }
    else if(strcmp(input, "song") == 0) {
        command_song(args);
    }
    else if(strcmp(input, "draw") == 0) {
        command_draw();
    }
    else {
        terminal_write("\n");
        terminal_write(input);
        terminal_write(" is not a command, write help for avalible commands\n");
    }
}