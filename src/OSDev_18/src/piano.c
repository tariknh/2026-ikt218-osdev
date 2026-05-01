#include <pianoApp/piano.h>
#include <kernel/terminal.h>
#include <kernel/pit.h>
#include <kernel/io.h>
#include <kernel/memory.h>
#include <kernel/keyboard.h>
#include <libc/stdint.h>

struct PianoAppState* CreatePiano(void) {
    struct PianoAppState* piano = (struct PianoAppState*)malloc(sizeof(struct PianoAppState));
    if (!piano) {
        return 0;
    }

    piano->recording = false;
    piano->lastInput = 0;

    piano->lastNoteEndTick = 0;
    piano->activeFrequency = 0;

    piano->songLibrary = (struct SongLibrary*)malloc(sizeof(struct SongLibrary));
    if (!piano->songLibrary) {
        free(piano);
        return 0;
    }

    piano->songLibrary->songCount = 0;

    piano->songLibrary->songs = (struct Song*)malloc(sizeof(struct Song) * MAX_SONG_COUNT);
    if (!piano->songLibrary->songs) {
        free(piano->songLibrary);
        free(piano);
        return 0;
    }

    for (uint32_t i = 0; i < MAX_SONG_COUNT; i++) {
        piano->songLibrary->songs[i].notes =
            (struct Note*)malloc(sizeof(struct Note) * MAX_SONG_SIZE);

        if (!piano->songLibrary->songs[i].notes) {
            for (uint32_t j = 0; j < i; j++) {
                free(piano->songLibrary->songs[j].notes);
            }

            free(piano->songLibrary->songs);
            free(piano->songLibrary);
            free(piano);
            return 0;
        }

        piano->songLibrary->songs[i].length = 0;
    }

    return piano;
}

void DestroyPiano(struct PianoAppState* piano) {
    if (piano) {
        if (piano->songLibrary) {
            if (piano->songLibrary->songs) {
                for (uint32_t i = 0; i < MAX_SONG_COUNT; i++) {
                    if (piano->songLibrary->songs[i].notes) {
                        free(piano->songLibrary->songs[i].notes);
                    }
                }

                free(piano->songLibrary->songs);
            }

            free(piano->songLibrary);
        }

        free(piano);
    }
}

bool IsPianoKey(char input) {
    return input == 'z' || input == 's' || input == 'x' || input == 'd' ||
           input == 'c' || input == 'v' || input == 'g' || input == 'b' ||
           input == 'h' || input == 'n' || input == 'j' || input == 'm' ||
           input == ',';
}

void PianoEnableSpeaker(void) {
    uint8_t speakerState = InPortByte(PC_SPEAKER_PORT);
    OutPortByte(PC_SPEAKER_PORT, speakerState | 0x03);
}

void PianoDisableSpeaker(void) {
    uint8_t speakerState = InPortByte(PC_SPEAKER_PORT);
    OutPortByte(PC_SPEAKER_PORT, speakerState & 0xFC);
}

void PianoPlaySound(uint32_t frequency) {
    if (!frequency) {
        return;
    }

    uint32_t divisor = PIT_BASE_FREQ / frequency;

    OutPortByte(PIT_CMD_PORT, 0xB6);
    OutPortByte(PIT_CHANNEL2_PORT, divisor & 0xFF);
    OutPortByte(PIT_CHANNEL2_PORT, (divisor >> 8) & 0xFF);

    PianoEnableSpeaker();
}

void PianoStopSound(void) {
    PianoDisableSpeaker();
}

void RecordNote(struct PianoAppState* piano, uint32_t frequency, uint32_t duration) {
    if (!piano->recording) {
        return;
    }

    if (piano->songLibrary->songCount >= MAX_SONG_COUNT) {
        return;
    }

    struct Song* song =
        &piano->songLibrary->songs[piano->songLibrary->songCount];

    if (song->length >= MAX_SONG_SIZE) {
        piano->recording = false;
        piano->songLibrary->songCount++;
        return;
    }

    song->notes[song->length].frequency = frequency;
    song->notes[song->length].duration = duration;
    song->length++;
}

void PianoHandleInput(struct PianoAppState* piano) {
    switch (piano->lastInput) {
        case 'z':
            PianoPlaySound(C4);
            piano->activeFrequency = C4;
            break;

        case 's':
            PianoPlaySound(Cs4);
            piano->activeFrequency = Cs4;
            break;

        case 'x':
            PianoPlaySound(D4);
            piano->activeFrequency = D4;
            break;

        case 'd':
            PianoPlaySound(Ds4);
            piano->activeFrequency = Ds4;
            break;

        case 'c':
            PianoPlaySound(E4);
            piano->activeFrequency = E4;
            break;

        case 'v':
            PianoPlaySound(F4);
            piano->activeFrequency = F4;
            break;

        case 'g':
            PianoPlaySound(Fs4);
            piano->activeFrequency = Fs4;
            break;

        case 'b':
            PianoPlaySound(G4);
            piano->activeFrequency = G4;
            break;

        case 'h':
            PianoPlaySound(Gs4);
            piano->activeFrequency = Gs4;
            break;

        case 'n':
            PianoPlaySound(A4);
            piano->activeFrequency = A4;
            break;

        case 'j':
            PianoPlaySound(As4);
            piano->activeFrequency = As4;
            break;

        case 'm':
            PianoPlaySound(B4);
            piano->activeFrequency = B4;
            break;

        case ',':
            PianoPlaySound(C5);
            piano->activeFrequency = C5;
            break;

        case 'r':
            // Recording always writes into the current song slot. Starting a
            // recording resets that slot; stopping it commits the slot by
            // incrementing songCount so the next recording uses a new entry.
            if (!piano->recording) {
                if (piano->songLibrary->songCount < MAX_SONG_COUNT) {
                    struct Song* song =
                        &piano->songLibrary->songs[piano->songLibrary->songCount];

                    song->length = 0;
                    piano->activeFrequency = R;
                    piano->lastNoteEndTick = 0;
                    piano->recording = true;
                }
            } else {
                piano->recording = false;
                piano->songLibrary->songCount++;
                piano->activeFrequency = R;
                piano->lastNoteEndTick = 0;
            }
            break;
            
        case 'p':
            PlaySongFromLibrary(piano->songLibrary);
            break;

        default:
            break;
    }
}

void PlaySongFromLibrary(struct SongLibrary* songLibrary) {
    if (!songLibrary->songCount) {
        TerminalWriteString("No songs have been recorded yet.\n");
        return;
    }

    uint32_t songChoice = 0;
    uint32_t songIndex = 0;

    do {
        TerminalWriteString("You have ");
        TerminalWriteUInt(songLibrary->songCount);
        TerminalWriteString(" songs created.\n");
        TerminalWriteString("Write a song number from 1 to ");
        TerminalWriteUInt(songLibrary->songCount);
        TerminalWriteString(": ");

        if (!TerminalGetUInt(&songChoice)) {
            TerminalWriteString("Error... Number is not correctly written.\n");
            continue;
        }

        if (songChoice == 0 || songChoice > songLibrary->songCount) {
            TerminalWriteString("Error... Song number is out of range.\n");
            continue;
        }

        songIndex = songChoice - 1;
        TerminalWriteString("\n");
    } while (songChoice == 0 || songChoice > songLibrary->songCount);

    for (uint32_t i = 0; i < songLibrary->songs[songIndex].length; i++) {
        struct Note currentNote = songLibrary->songs[songIndex].notes[i];

        if (currentNote.frequency == R) {
            PianoStopSound();
            SleepInterrupt(currentNote.duration);
        } else {
            PianoPlaySound(currentNote.frequency);
            SleepInterrupt(currentNote.duration);
            PianoStopSound();
        }
    }

    PianoStopSound();
}

void DrawPianoUi(struct PianoAppState* piano) {
    uint32_t songCount = piano->songLibrary->songCount;

    TerminalWriteString("============================================================\n");
    TerminalWriteString("                        PC SPEAKER PIANO                    \n");
    TerminalWriteString("============================================================\n\n");

    TerminalWriteString("Controls:");
    TerminalWriteString("[r] Record   [p] Playback   [q] Quit\n\n");

    TerminalWriteString("           +-----+-----+     +-----+-----+-----+\n");
    TerminalWriteString("           [  s  I  d  ]     [  g  I  h  I  j  ]\n");
    TerminalWriteString("           [ Cs4 I Ds4 ]     [ Fs4 I Gs4 I As4 ]\n");
    TerminalWriteString("           +-----+-----+     +-----+-----+-----+\n\n");

    TerminalWriteString("        +-----+-----+-----+-----+-----+-----+-----+-----+\n");
    TerminalWriteString("        [  z  I  x  I  c  I  v  I  b  I  n  I  m  I  ,  ]\n");
    TerminalWriteString("        [ C4  I D4  I E4  I F4  I G4  I A4  I B4  I C5  ]\n");
    TerminalWriteString("        +-----+-----+-----+-----+-----+-----+-----+-----+\n\n");

    TerminalWriteString(piano->recording ? "Recording: ON\n" : "Recording: OFF\n");

    TerminalWriteString("Stored notes: ");
    if (piano->recording) {
        TerminalWriteUInt(piano->songLibrary->songs[songCount].length);
    } else {
        TerminalWriteUInt(0);
    }

    TerminalWriteString(" / ");
    TerminalWriteUInt(MAX_SONG_SIZE);
    TerminalWriteString("\n");

    TerminalWriteString("Last note: ");

    if (piano->lastInput != 0) {
        TerminalPutChar(piano->lastInput);
    } else {
        TerminalWriteString("-");
    }

    TerminalWriteString("\n");
}

void PlayPiano(void) {
    struct PianoAppState* piano = CreatePiano();
    if (!piano) {
        TerminalWriteString("Failed to create piano.\n");
        return;
    }

    TerminalClear();
    DrawPianoUi(piano);

    while (1) {
        char input = GetLastKeyPressed();

        if (!input) {
            SleepInterrupt(1);
            continue;
        }

        if (input == 'q') {
            PianoStopSound();
            break;
        }

        if (input == 'r' || input == 'p' || input == 'e') {
            piano->lastInput = input;
            PianoHandleInput(piano);

            TerminalClear();
            DrawPianoUi(piano);
            continue;
        }

        if (!IsPianoKey(input)) {
            continue;
        }

        piano->lastInput = input;

        if (piano->recording && piano->lastNoteEndTick != 0) {
            uint32_t now = GetCurrentTick();
            uint32_t restDuration = now - piano->lastNoteEndTick;

            // Store longer gaps as explicit rest notes so playback preserves
            // rhythm instead of compressing every pause between key presses.
            if (restDuration > 20) {
                RecordNote(piano, R, restDuration);
            }
        }

        PianoHandleInput(piano);

        if (piano->recording) {
            RecordNote(piano, piano->activeFrequency, PIANO_NOTE_DURATION);
        }

        SleepInterrupt(PIANO_NOTE_DURATION);
        PianoStopSound();

        piano->lastNoteEndTick = GetCurrentTick();

        TerminalClear();
        DrawPianoUi(piano);
    }

    DestroyPiano(piano);
}
