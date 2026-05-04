#ifndef SONG_H
#define SONG_H

#include <libc/stdint.h>
#include <libc/stddef.h>
#include <song/frequencies.h>

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

/*
 * Simple test melody.
 */
static Note music_1[] = {
    {NOTE_C4, 300},
    {NOTE_D4, 300},
    {NOTE_E4, 300},
    {NOTE_F4, 300},
    {NOTE_G4, 500},
    {NOTE_REST, 150},
    {NOTE_G4, 300},
    {NOTE_F4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 300},
    {NOTE_C4, 500}
};

#endif
