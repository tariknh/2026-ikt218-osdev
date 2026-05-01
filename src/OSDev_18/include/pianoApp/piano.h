#ifndef PIANOAPP_PIANO_H
#define PIANOAPP_PIANO_H

#include "pianoApp/frequencies.h"
#include <libc/stdint.h>
#include <libc/stdbool.h>

#define MAX_SONG_SIZE 256
#define MAX_SONG_COUNT 8

#define PIANO_NOTE_DURATION 200

struct Note {
    uint32_t frequency;
    uint32_t duration;
};

struct Song {
    struct Note* notes;
    uint32_t length;
};

struct SongLibrary {
    struct Song* songs;
    uint32_t songCount;
};

struct PianoAppState {
    struct SongLibrary* songLibrary;
    bool recording;
    char lastInput;

    uint32_t activeFrequency;
    uint32_t lastNoteEndTick;
};

struct PianoAppState* CreatePiano(void);
void DestroyPiano(struct PianoAppState* piano);

void PianoEnableSpeaker(void);
void PianoDisableSpeaker(void);

void PianoPlaySound(uint32_t frequency);
void PianoStopSound(void);

void PianoHandleInput(struct PianoAppState* piano);

void PlaySongFromLibrary(struct SongLibrary* songLibrary);
void RecordNote(struct PianoAppState* piano, uint32_t frequency, uint32_t duration);

void DrawPianoUi(struct PianoAppState* piano);

void PlayPiano(void);

#endif