#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "memory.h"
#include "pit.h"
#include "song.h"

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

    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)},
        {starwars_theme, sizeof(starwars_theme) / sizeof(Note)},
        {battlefield_1942_theme, sizeof(battlefield_1942_theme) / sizeof(Note)},
        {music_2, sizeof(music_2) / sizeof(Note)},
        {music_3, sizeof(music_3) / sizeof(Note)},
        {music_4, sizeof(music_4) / sizeof(Note)},
        {music_5, sizeof(music_5) / sizeof(Note)},
        {music_6, sizeof(music_6) / sizeof(Note)},
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player();

    while(1) {
        for (uint32_t i = 0; i < n_songs; i++) {
            printf("Playing Song...\n");
            player->play_song(&songs[i]);
            printf("Finished playing the song.\n");
        }
    }
}