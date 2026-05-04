/*
 * songs.c - Built-in song data
 *
 * music_1 is "Twinkle, Twinkle, Little Star" - the first 14 notes of the
 * melody, played at A=440 Hz reference pitch.  Each note is given a
 * duration in milliseconds.
 *
 * Reference frequencies (all in 4th octave):
 *    C  = 262   D = 294   E = 330   F = 349
 *    G  = 392   A = 440   B = 494
 */

#include <song.h>
#include <libc/stdint.h>
#include <libc/stddef.h>

Note music_1[] = {
    /* Twinkle twinkle */
    { 262, 400 }, { 262, 400 }, { 392, 400 }, { 392, 400 },
    /* little star */
    { 440, 400 }, { 440, 400 }, { 392, 800 },
    /*   0,  100 = a brief rest between phrases */
    {   0, 100 },
    /* How I wonder */
    { 349, 400 }, { 349, 400 }, { 330, 400 }, { 330, 400 },
    /* what you are */
    { 294, 400 }, { 294, 400 }, { 262, 800 },
};

size_t music_1_length = sizeof(music_1) / sizeof(Note);
