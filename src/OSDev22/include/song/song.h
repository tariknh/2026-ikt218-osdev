#ifndef SONG_H
#define SONG_H

#include "libc/stdint.h"
#include "song/frequencies.h"

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} Note;

typedef struct {
    Note* notes;
    uint32_t length;
} Song;

typedef struct {
    void (*play_song)(Song* song);
} SongPlayer;

SongPlayer* create_song_player();

static Note music_1[] = {
    {E5, 250}, {R, 125}, {E5, 125}, {R, 125}, {E5, 125}, {R, 125},
    {C5, 125}, {E5, 125}, {G5, 125}, {R, 125}, {G4, 125}, {R, 250},

    {C5, 125}, {R, 250}, {G4, 125}, {R, 125}, {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125}, {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125}, {F5, 125}, {G5, 125},
    {R, 125}, {E5, 125}, {C5, 125}, {D5, 125}, {B4, 125}, {R, 125},
};

#endif