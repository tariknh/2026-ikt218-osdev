#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "memory.h"
#include "pit.h"
#include "song.h"
#include "snake.h"

extern uint32_t end;

SongPlayer* create_song_player() {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    player->play_song = play_song_impl;
    return player;
}

void main(uint32_t mb_magic, void* mb_info) {
    (void)mb_magic; //stop warnings about unused variables
    (void)mb_info;

    terminal_initialize();
    init_gdt();
    init_idt();
    init_irq();
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    __asm__ volatile ("sti"); //enable interrupts after PIC is remapped

    printf("Hello World\n");

    SongPlayer* player = create_song_player();
    Song demo_song = { music_1, sizeof(music_1) / sizeof(Note) };
    printf("Playing demo song...\n");
    player->play_song(&demo_song);
    printf("Finished playing the song.\n");
    free(player);

    printf("Starting Snake Game...\n");
    sleep_interrupt(1000);

    snake_game(); //this never returns
}
