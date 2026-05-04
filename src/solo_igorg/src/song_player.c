#include <song_player.h>
#include <song/song.h>
#include <pit.h>
#include <io.h>
#include <memory.h>
#include <terminal.h>
#include <libc/stdint.h>
#include <libc/stddef.h>

/*
 * PC speaker and PIT ports.
 * Connects PIT channel 2 to PC speaker.
 */
#define PC_SPEAKER_PORT 0x61
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL2_PORT 0x42
#define PIT_BASE_FREQUENCY 1193182

static void enable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);

    /*
     * Set bit 0 and bit 1:
     * bit 0 = speaker gate
     * bit 1 = speaker data
     */
    if ((state & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, state | 0x03);
    }
}

static void disable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);

    /*
     * Clears bit 0 and bit 1 to disable the speaker.
     */
    outb(PC_SPEAKER_PORT, state & 0xFC);
}

static void play_sound(uint32_t frequency)
{
    if (frequency == 0) {
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    /*
     * Command byte 0xB6:
     *  - channel 2
     *  - access mode: low byte/high byte
     *  - mode 3: square wave generator
     *  - binary mode
     */
    outb(PIT_COMMAND_PORT, 0xB6);

    /*
     * Sends divisor low byte, then high byte to PIT channel 2.
     */
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

static void stop_sound(void)
{
    disable_speaker();
}

void play_song_impl(Song* song)
{
    if (song == 0 || song->notes == 0) {
        return;
    }

    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];

        if (note.frequency == 0) {
            stop_sound();
        } else {
            play_sound(note.frequency);
        }

        sleep_interrupt(note.duration);

        /*
         * Pause between notes so they do not blend together.
         */
        stop_sound();
        sleep_interrupt(20);
    }

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

void play_music(void)
{
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)}
    };

    uint32_t number_of_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player();

    if (player == 0) {
        terminal_write("Could not create song player\n");
        return;
    }

    for (uint32_t i = 0; i < number_of_songs; i++) {
        terminal_write("Playing song...\n");
        player->play_song(&songs[i]);
        terminal_write("Finished playing song.\n");
    }
}
