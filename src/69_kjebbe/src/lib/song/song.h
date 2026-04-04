#ifndef SONG_H
#define SONG_H
#ifdef __cplusplus
extern "C" {
#endif
#include "../../../include/kernel/memory.h"
#ifdef __cplusplus
}
#endif
#include "../../../include/libc/stdint.h"
#include "frequencies.h"

// Define a struct to represent a single musical note
typedef struct {
  uint32_t frequency; // The frequency of the note in Hz (e.g., A4 = 440 Hz)
  uint32_t duration;  // The duration of the note in milliseconds
} Note;

// Define a struct to represent a song
typedef struct {
  Note *notes;     // Pointer to an array of Note structs representing the song
  uint32_t length; // The number of notes in the song
} Song;

// Define a struct to represent a song player
typedef struct {
  void (*play_song)(
      Song *song); // Function pointer to a function that plays a song
} SongPlayer;

// Function prototype for creating a new SongPlayer instance
// Returns a pointer to a newly created SongPlayer object
#ifdef __cplusplus
extern "C"
#endif
    SongPlayer *create_song_player();

#ifdef __cplusplus
extern "C"
#endif
    void piano_play_sound(uint32_t frequency);

#ifdef __cplusplus
extern "C"
#endif
    void disable_speaker();

static Note music_1[] = {
    {E5, 250}, {R, 125},  {E5, 125}, {R, 125},        {E5, 125}, {R, 125},
    {C5, 125}, {E5, 125}, {G5, 125}, {R, 125},        {G4, 125}, {R, 250},

    {C5, 125}, {R, 250},  {G4, 125}, {R, 125},        {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125},  {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125},       {F5, 125}, {G5, 125},
    {R, 125},  {E5, 125}, {C5, 125}, {D5, 125},       {B4, 125}, {R, 125},

    {C5, 125}, {R, 250},  {G4, 125}, {R, 125},        {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125},  {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125},       {F5, 125}, {G5, 125},
    {R, 125},  {E5, 125}, {C5, 125}, {D5, 125},       {B4, 125}, {R, 125},
};

static Note music_2[] = {
    {A4, 200},  {E5, 200}, {A5, 200},  {R, 100},  {A5, 200},  {A5, 200},
    {Gs5, 200}, {A5, 200}, {R, 100},   {E5, 200}, {R, 100},   {E5, 200},
    {R, 100},   {E5, 200}, {R, 100},   {E5, 200}, {A4, 200},  {E5, 200},
    {A5, 200},  {R, 100},  {A5, 200},  {A5, 200}, {Gs5, 200}, {A5, 200},
    {R, 100},   {E5, 200}, {R, 100},   {E5, 200}, {R, 100},   {E5, 200},
    {R, 100},   {E5, 200}, {A4, 200},  {E5, 200}, {A5, 200},  {R, 100},
    {A5, 200},  {A5, 200}, {Gs5, 200}, {A5, 200}, {R, 100},   {E5, 200},
    {R, 100},   {E5, 200}, {R, 100},   {E5, 200}, {R, 100},   {E5, 200}};

static Note music_3[] = {
    {E4, 200}, {E4, 200}, {F4, 200}, {G4, 200}, {G4, 200}, {F4, 200}, {E4, 200},
    {D4, 200}, {C4, 200}, {C4, 200}, {D4, 200}, {E4, 200}, {E4, 400}, {R, 200},
    {D4, 200}, {D4, 200}, {E4, 200}, {F4, 200}, {F4, 200}, {E4, 200}, {D4, 200},
    {C4, 200}, {A4, 200}, {A4, 200}, {A4, 200}, {G4, 400}};

static Note music_4[] = {
    {C4, 500}, {D4, 500},  {E4, 500}, {C4, 500}, {C4, 500},  {D4, 500},
    {E4, 500}, {C4, 500},  {E4, 500}, {F4, 500}, {G4, 1000}, {E4, 500},
    {F4, 500}, {G4, 1000}, {G4, 250}, {A4, 250}, {G4, 250},  {F4, 250},
    {E4, 500}, {C4, 500},  {G4, 250}, {A4, 250}, {G4, 250},  {F4, 250},
    {E4, 500}, {C4, 500},  {C4, 500}, {G3, 500}, {C4, 1000}, {C4, 500},
    {G3, 500}, {C4, 1000}};

static Note music_5[] = {
    {E4, 375}, {C4, 375}, {D4, 375}, {A3, 375}, {B3, 375}, {D4, 375},
    {C4, 375}, {A3, 375}, {E4, 375}, {C4, 375}, {D4, 375}, {A3, 375},
    {B3, 375}, {D4, 375}, {C4, 375}, {A3, 375},
};

static Note music_6[] = {
    {F4, 250},       {F4, 250},       {F4, 250},       {C5, 250},
    {A_SHARP4, 250}, {G_SHARP4, 250}, {F4, 500},       {F4, 250},
    {F4, 250},       {F4, 250},       {C5, 250},       {A_SHARP4, 250},
    {G_SHARP4, 250}, {F4, 500},       {A_SHARP4, 250}, {A_SHARP4, 250},
    {A_SHARP4, 250}, {F5, 250},       {D5, 250},       {C5, 250},
    {A_SHARP4, 500}, {A_SHARP4, 250}, {A_SHARP4, 250}, {A_SHARP4, 250},
    {F5, 250},       {D5, 250},       {C5, 250},       {A_SHARP4, 500},
};

// Defines musical durations
static const int bar = 3000;
static const int half = bar / 2;
static const int quarter = bar / 4;
static const int eight = bar / 8;
static const int sixteenth = bar / 16;
static const int half_dot = bar / 4 + quarter;
static const int quarter_dot = bar / 4 + eight;
static const int eight_dot = bar / 4 + sixteenth;

static Note nasjonal_sangen[] = {
    // bar 1
    {G4, quarter_dot},
    {F4, eight},
    {E4, quarter},
    {D4, quarter},
    // bar 2
    {C4, quarter},
    {D4, quarter},
    {E4, quarter},
    {F4, quarter},
    // bar 3
    {G4, quarter_dot},
    {A4, eight},
    {G4, quarter},
    {F4, quarter},
    // bar 4
    {E4, half_dot},
    {R, quarter},
    // bar 5
    {A4, quarter_dot},
    {G4, eight},
    {F4, quarter},
    {E4, quarter},
    // bar 6
    {D4, quarter},
    {E4, quarter},
    {F4, quarter},
    {G4, quarter},
    // bar 7
    {G4, quarter_dot},
    {A4, eight},
    {A4, quarter},
    {B4, quarter},
    // bar 8
    {C5, half},
    {R, half},
    // bar 9
    {C5, quarter_dot},
    {C5, eight},
    {B4, eight_dot},
    {B4, sixteenth},
    {A4, eight_dot},
    {A4, sixteenth},
    // bar 10
    {G4, half},
    {E4, quarter},
    {R, quarter},
    // bar 11
    {A4, quarter_dot},
    {A4, eight},
    {G4, quarter},
    {G4, quarter},
    // bar 12
    {F4, half},
    {R, quarter},
    {G4, eight_dot},
    {G4, sixteenth},
    // bar 13
    {G4, quarter},
    {A4, quarter},
    {A4, quarter},
    {B4, quarter},
    // bar 14
    {B4, half},
    {C5, half},
    // bar 15
    {C5, quarter_dot},
    {C5, eight},
    {B4, quarter},
    {C5, quarter},
    // bar 16
    {D5, half},
    {R, quarter},
    {D5, eight_dot},
    {D5, sixteenth},
    // bar 17
    {D5, quarter},
    {E5, quarter},
    {F5, quarter},
    {E5, quarter},
    // bar 18
    {D5, half},
    {C5, quarter},
    {B4, eight_dot},
    {A4, sixteenth},
    // bar 19
    {G4, quarter_dot},
    {A4, eight},
    {A4, quarter},
    {B4, quarter},
    // bar 20
    {C5, bar},

};

// Removes a duration at the end of each note and replaces it silence of the
// same duration.
static Song add_space_between_notes(Song *song, uint32_t breath_ms) {
  Note *new_notes = (Note *)malloc(sizeof(Note) * song->length * 2);
  for (uint32_t i = 0; i < song->length; i++) {
    new_notes[i * 2] =
        (Note){song->notes[i].frequency, song->notes[i].duration - breath_ms};
    new_notes[i * 2 + 1] = (Note){R, breath_ms};
  }
  return (Song){new_notes, song->length * 2};
}

#endif
