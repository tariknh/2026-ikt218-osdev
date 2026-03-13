#include "song/song.h"
#include "drivers/speaker.h"
#include "kernel/pit.h"
#include "libc/stdio.h"
#include "stdint.h"
#include "kernel/memory.h"

void play_song_impl(Song *song) {
    if (song == 0 || song->notes == 0 || song->length == 0) {
        printf("No song data.\n");
        return;
    }

    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];

        printf("Note %u: freq=%u duration=%u\n", i, note.frequency, note.duration);

        if (note.frequency == 0) {
            stop_sound();
        } else {
            play_sound(note.frequency);
        }

        sleep_interrupt(note.duration);
        stop_sound();
        sleep_interrupt(20);
    }

    disable_speaker();
}

SongPlayer* create_song_player() {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    if (player == 0) {
        printf("Failed to allocate SongPlayer.\n");
        return 0;
    }

    player->play_song = play_song_impl;
    return player;
}
