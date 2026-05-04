#ifndef SONG_H
#define SONG_H

#include "frequencies.h"
#include "libc/stdint.h"

// Represents one musical note
typedef struct {
    uint32_t frequency; // Frequency of the note in Hz
    uint32_t duration; // Duration of the note in milliseconds
} Note;

// Represents a full song
typedef struct {
    Note* notes; // Pointer to an array of notes
    uint32_t length; // Number of notes in the song
} Song;

// Simple song player interface
typedef struct {
    void (*play_song)(Song* song); // Function pointer for playing a song
} SongPlayer;

// Creates and returns a song player
SongPlayer* create_song_player();
// Actual implementation for playing a song
void play_song_impl(Song* song);
// Wrapper function for playing a song
void play_song(Song* song);

// Mario-like melody
static Note music_1[] = {
    {E5, 250}, {R, 125}, {E5, 125}, {R, 125}, {E5, 125}, {R, 125},
    {C5, 125}, {E5, 125}, {G5, 125}, {R, 125}, {G4, 125}, {R, 250},

    {C5, 125}, {R, 250}, {G4, 125}, {R, 125}, {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125}, {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125}, {F5, 125}, {G5, 125},
    {R, 125}, {E5, 125}, {C5, 125}, {D5, 125}, {B4, 125}, {R, 125},

    {C5, 125}, {R, 250}, {G4, 125}, {R, 125}, {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125}, {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125}, {F5, 125}, {G5, 125},
    {R, 125}, {E5, 125}, {C5, 125}, {D5, 125}, {B4, 125}, {R, 125},
};

// Star Wars theme melody
static Note starwars_theme[] = {
    {A4, 500}, {A4, 500}, {A4, 500},
    {F4, 375}, {C5, 125},
    {A4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {E5, 500}, {E5, 500}, {E5, 500},
    {F5, 375}, {C5, 125},

    {G4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {A5, 500}, {A4, 375}, {A4, 125},
    {A5, 500}, {G5, 375}, {F5, 125}, {E5, 125}, {D5, 125},
    {C5, 250}, {B4, 250}, {A4, 500},

    {R, 500} // Pause at the end
};
// Battlefield 1942 theme melody
static Note battlefield_1942_theme[] = {
    {E4, 500}, {G4, 500}, {B4, 300}, {E5, 200},
    {D5, 200}, {B4, 300}, {G4, 500}, {B4, 300},
    {E5, 200}, {D5, 200}, {B4, 300}, {G4, 500},
    {B4, 300}, {E5, 200}, {G5, 200}, {E5, 300},

    {D5, 200}, {B4, 300}, {G4, 500}, {E4, 500},
    {G4, 500}, {B4, 300}, {E5, 200}, {D5, 200},
    {B4, 300}, {G4, 500}, {B4, 300}, {E5, 200},
    {D5, 200}, {B4, 300}, {G4, 500}, {B4, 300},
    {E5, 200}, {G5, 200}, {E5, 300}, {D5, 200},
    {B4, 300}, {G4, 500},

    {R, 500} // Pause at the end
};
// Custom melody 2
static Note music_2[] = {
    {A4, 200}, {E5, 200}, {A5, 200}, {R, 100}, {A5, 200}, {A5, 200}, {G_SHARP4, 200}, {A5, 200},
    {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200},
    {A4, 200}, {E5, 200}, {A5, 200}, {R, 100}, {A5, 200}, {A5, 200}, {G_SHARP4, 200}, {A5, 200},
    {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200},
    {A4, 200}, {E5, 200}, {A5, 200}, {R, 100}, {A5, 200}, {A5, 200}, {G_SHARP4, 200}, {A5, 200},
    {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200}, {R, 100}, {E5, 200}
};
// Custom melody 3
static Note music_3[] = {
    {E4, 200}, {E4, 200}, {F4, 200}, {G4, 200}, {G4, 200}, {F4, 200}, {E4, 200}, {D4, 200},
    {C4, 200}, {C4, 200}, {D4, 200}, {E4, 200}, {E4, 400}, {R, 200},
    {D4, 200}, {D4, 200}, {E4, 200}, {F4, 200}, {F4, 200}, {E4, 200}, {D4, 200}, {C4, 200},
    {A4, 200}, {A4, 200}, {A4, 200}, {G4, 400}
};
// Custom melody 4
static Note music_4[] = {
    {C4, 500}, {D4, 500}, {E4, 500}, {C4, 500},
    {C4, 500}, {D4, 500}, {E4, 500}, {C4, 500},
    {E4, 500}, {F4, 500}, {G4, 1000},
    {E4, 500}, {F4, 500}, {G4, 1000},
    {G4, 250}, {A4, 250}, {G4, 250}, {F4, 250}, {E4, 500}, {C4, 500},
    {G4, 250}, {A4, 250}, {G4, 250}, {F4, 250}, {E4, 500}, {C4, 500},
    {C4, 500}, {G3, 500}, {C4, 1000},
    {C4, 500}, {G3, 500}, {C4, 1000}
};
// Custom melody 5
static Note music_5[] = {
    {E4, 375}, {C4, 375}, {D4, 375}, {A3, 375}, {B3, 375}, {D4, 375}, {C4, 375}, {A3, 375},
    {E4, 375}, {C4, 375}, {D4, 375}, {A3, 375}, {B3, 375}, {D4, 375}, {C4, 375}, {A3, 375},
};
// Custom melody 6
static Note music_6[] = {
    {F4, 250}, {F4, 250}, {F4, 250}, {C5, 250}, {A_SHARP4, 250}, {G_SHARP4, 250}, {F4, 500},
    {F4, 250}, {F4, 250}, {F4, 250}, {C5, 250}, {A_SHARP4, 250}, {G_SHARP4, 250}, {F4, 500},
    {A_SHARP4, 250}, {A_SHARP4, 250}, {A_SHARP4, 250}, {F5, 250}, {D5, 250}, {C5, 250}, {A_SHARP4, 500},
    {A_SHARP4, 250}, {A_SHARP4, 250}, {A_SHARP4, 250}, {F5, 250}, {D5, 250}, {C5, 250}, {A_SHARP4, 500},
};

#endif