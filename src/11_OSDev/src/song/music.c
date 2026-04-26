#include "song/song.h"

/* Note frequencies (Hz) */
#define E5  659
#define D5  587
#define C5  523
#define B4  494
#define A4  440
#define F5  698
#define G5  784
#define A5  880
#define REST 0

/* Duration macros (ms) at 150 BPM */
#define QN  400   /* quarter note */
#define EN  200   /* eighth note  */
#define DQN 600   /* dotted quarter */
#define HN  800   /* half note */

Note music_1[] = {
    /* part A*/
    {E5,  QN},
    {B4,  EN},
    {C5,  EN},
    {D5,  QN},
    {C5,  EN},
    {B4,  EN},
    {A4,  QN},
    {A4,  EN},
    {C5,  EN},
    {E5,  QN},
    {D5,  EN},
    {C5,  EN},
    {B4,  DQN},
    {C5,  EN},
    {D5,  QN},
    {E5,  QN},
    {C5,  QN},
    {A4,  QN},
    {A4,  HN},
    /* Part B */
    {REST, EN},
    {D5,  DQN},
    {F5,  EN},
    {A5,  QN},
    {G5,  EN},
    {F5,  EN},
    {E5,  DQN},
    {C5,  EN},
    {E5,  QN},
    {D5,  EN},
    {C5,  EN},
    {B4,  QN},
    {B4,  EN},
    {C5,  EN},
    {D5,  QN},
    {E5,  QN},
    {C5,  QN},
    {A4,  QN},
    {A4,  QN},
    {REST, QN},
};

uint32_t music_1_count = sizeof(music_1) / sizeof(Note);
