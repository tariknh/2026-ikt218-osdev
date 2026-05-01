#ifndef SONG_H
#define SONG_H

#include "libc/system.h"
#include "song/frequencies.h"

// Define a struct to represent a single musical note
typedef struct {
    uint32_t frequency; // Hz
    uint32_t duration;  // ms
} Note;

// Define a struct to represent a song
typedef struct {
    Note* notes;
    uint32_t length;
} Song;

// Prototypes
void play_song_impl(Song *song);
void play_song(Song *song);

// This is what kernel.c calls:
void play_music(void);

// Expose one default song (defined in src/song.c)
extern Note music_1[];
extern uint32_t music_1_len;

#endif