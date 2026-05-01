#include "memory.h"
#include "pit.h"
#include "ports.h"
#include "song.h"
#include "terminal.h"

void enable_speaker(void)
{
    uint8_t speaker_state = port_byte_in(PC_SPEAKER_PORT);
    port_byte_out(PC_SPEAKER_PORT, speaker_state | 0x03);
}

void disable_speaker(void)
{
    uint8_t speaker_state = port_byte_in(PC_SPEAKER_PORT);
    port_byte_out(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

void play_sound(uint32_t frequency)
{
    if (frequency == 0) {
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    port_byte_out(PIT_CMD_PORT, 0xB6);
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8));

    enable_speaker();
}

void stop_sound(void)
{
    disable_speaker();
}

SongPlayer* create_song_player(void)
{
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));

    if (player == 0) {
        return 0;
    }

    player->play_song = play_song_impl;
    return player;
}

void play_song_impl(Song *song)
{
    if (song == 0 || song->notes == 0) {
        terminal_write("No song to play\n");
        return;
    }

    terminal_write("Playing song...\n");

    for (uint32_t i = 0; i < song->length; i++) {
        play_sound(song->notes[i].frequency);
        sleep_interrupt(song->notes[i].duration);
        stop_sound();
    }

    disable_speaker();

    terminal_write("Finished playing song\n");
}
