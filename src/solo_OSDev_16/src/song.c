#include "song/song.h"
#include "pit.h"
#include "io.h"
#include "memory.h"

void enable_speaker() {
    // Read the current state of the PC speaker control register
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);

        // If bits 0 and 1 are not set to 1, enable the speaker by setting both bits to 1
        if (speaker_state != (speaker_state | 3)) {
        outb(PC_SPEAKER_PORT, speaker_state | 3);
    }
}

void disable_speaker() {
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    // 0xFC is used as a mask to clear bit 0 and bit 1.
    outb(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

void play_sound(uint32_t frequency){
    if (frequency == 0){
        stop_sound();
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    // Program PIT channel 2 for PC speaker output
    outb(PIT_CMD_PORT, 0b10110110);      // Giving instructions on format of the transmission to channel 2
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF)); // Sending the lower bits, the PIT stores them
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor >> 8));   // Shift the upper 8 bits into position, then send them as the high byte
    
    enable_speaker();
}

void stop_sound(){
    uint8_t speaker_state = inb(PC_SPEAKER_PORT); 
    // 0xFD is used as a mask to clear bit 1.
    outb(PC_SPEAKER_PORT, speaker_state & 0xFD);    
}

void play_song_impl(Song *song){
    enable_speaker();
    for(uint32_t i = 0; i < song->length; i++){
        Note *note = &song->notes[i];       // Get the current note from the song's notes array
        play_sound(note->frequency);        // Play the note frequency       
        sleep_interrupt(note->duration);    // wait for the notes duration
        stop_sound();
    }
    disable_speaker();
}

void play_song(Song *song){
    play_song_impl(song);
}

Song music_1 = {
    music_1_notes,
    sizeof(music_1_notes) / sizeof(Note)
};

Song music_2 = {
    music_2_notes,
    sizeof(music_2_notes) / sizeof(Note)
};

Song music_3 = {
    music_3_notes,
    sizeof(music_3_notes) / sizeof(Note)
};

Song music_4 = {
    music_4_notes,
    sizeof(music_4_notes) / sizeof(Note)
};

Song music_5 = {
    music_5_notes,
    sizeof(music_5_notes) / sizeof(Note)
};

Song music_6 = {
    music_6_notes,
    sizeof(music_6_notes) / sizeof(Note)
};
