#include <songApp/song.h>
#include <kernel/pit.h>
#include <kernel/io.h>
#include <kernel/memory.h>
#include <libc/stdint.h>

void EnableSpeaker(void) {
    uint8_t speakerState = InPortByte(PC_SPEAKER_PORT);
    OutPortByte(PC_SPEAKER_PORT, speakerState | 0x03);
}

void DisableSpeaker(void) {
    uint8_t speakerState = InPortByte(PC_SPEAKER_PORT);
    OutPortByte(PC_SPEAKER_PORT, speakerState & 0xFC);
}

void PlaySound(uint32_t frequency) {
    if (!frequency) return;

    uint32_t divisor = PIT_BASE_FREQ / frequency;

    OutPortByte(PIT_CMD_PORT, 0xB6);

    OutPortByte(PIT_CHANNEL2_PORT, divisor & 0xFF);
    OutPortByte(PIT_CHANNEL2_PORT, (divisor >> 8) & 0xFF);

    EnableSpeaker();
}

void StopSound(void) {
    DisableSpeaker();
}

void PlaySongImpl(Song* song) {
    for (uint32_t i = 0; i < song->length; i++) {
        Note currentNote = song->notes[i];

        if (currentNote.frequency == R) {
            StopSound();
            SleepInterrupt(currentNote.duration);
        } else {
            PlaySound(currentNote.frequency);
            SleepInterrupt(currentNote.duration);
            StopSound();
        }
    }
}

SongPlayer* CreateSongPlayer(void) {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    if (!player) return 0;
    
    player->play_song = PlaySongImpl;
    return player;
}