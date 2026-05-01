#ifndef UIAOS_SONG_H
#define UIAOS_SONG_H

#include <stdint.h>
#include <stddef.h>
#include <song/frequencies.h>

typedef struct Note {
    uint32_t frequency;
    uint32_t duration;
} Note;

typedef struct Song {
    Note *notes;
    uint32_t length;
} Song;

typedef struct SongPlayer {
    void (*play_song)(Song *song);
} SongPlayer;

SongPlayer *create_song_player(void);
void play_song_impl(Song *song);
void play_music(void);

static Note music_1[] = {
    {E5, 250}, {R, 125}, {E5, 125}, {R, 125}, {E5, 125}, {R, 125},
    {C5, 125}, {E5, 125}, {G5, 125}, {R, 125}, {G4, 125}, {R, 250},
    {C5, 125}, {R, 250}, {G4, 125}, {R, 125}, {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125}, {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125}, {F5, 125}, {G5, 125},
    {R, 125}, {E5, 125}, {C5, 125}, {D5, 125}, {B4, 125}, {R, 125},
};

static Note starwars_theme[] = {
    {A4, 500}, {A4, 500}, {A4, 500},
    {F4, 375}, {C5, 125},
    {A4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {E5, 500}, {E5, 500}, {E5, 500},
    {F5, 375}, {C5, 125},
    {G4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {R, 500}
};

#endif
