#include <stdint.h>
#include <stddef.h>
#include <song/song.h>
#include <kernel/speaker.h>
#include <kernel/pit.h>
#include <kernel/memory.h>
#include <kernel/terminal.h>

SongPlayer *create_song_player(void)
{
    SongPlayer *player = (SongPlayer *)malloc(sizeof(SongPlayer));
    if (player == NULL) {
        return NULL;
    }

    player->play_song = play_song_impl;
    return player;
}

void play_song_impl(Song *song)
{
    if (song == NULL || song->notes == NULL) {
        return;
    }

    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];

        if (note.frequency == 0) {
            stop_sound();
        } else {
            play_sound(note.frequency);
        }

        sleep_interrupt(note.duration);
        stop_sound();
        sleep_busy(20);
    }

    disable_speaker();
}

void play_music(void)
{
    Song songs[] = {
        { music_1, sizeof(music_1) / sizeof(Note) },
        { starwars_theme, sizeof(starwars_theme) / sizeof(Note) }
    };

    SongPlayer *player = create_song_player();
    if (player == NULL) {
        printf("[MUSIC] Failed to create song player.\n");
        return;
    }

    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    for (uint32_t i = 0; i < n_songs; i++) {
        printf("[MUSIC] Playing song %d/%d...\n", i + 1, n_songs);
        player->play_song(&songs[i]);
        printf("[MUSIC] Finished song %d.\n", i + 1);
        sleep_interrupt(500);
    }

    free(player);
    disable_speaker();
}
