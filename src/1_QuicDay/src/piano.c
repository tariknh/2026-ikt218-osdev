#include "piano.h"
#include "libc/stdint.h"
#include "pit.h"

// Globale port I/O-implementasjoner for piano-filer
void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}


//Play sound using built-in speaker
void piano_play_sound(uint32_t nFrequence) {
 	uint32_t Div;
 	uint8_t tmp;
 
        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t) (Div) );
 	outb(0x42, (uint8_t) (Div >> 8));
 
        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(0x61, tmp | 3);
 	}
 }
 
 //make it shut up
void nosound(void) {
 	uint8_t tmp = inb(0x61) & 0xFC;
     
 	outb(0x61, tmp);
 }


 // Enkel piano-funksjon: trykk 1-8 for å spille toner
void piano_play_sound_keys(void) {
    while (1) {
        if (inb(0x64) & 0x01) { // Data klar fra tastatur
            uint8_t scancode = inb(0x60);
                switch (scancode) {
                    case 0x02: piano_play_sound(NOTE_C4); break; // 1
                    case 0x03: piano_play_sound(NOTE_D4); break; // 2
                    case 0x04: piano_play_sound(NOTE_E4); break; // 3
                    case 0x05: piano_play_sound(NOTE_F4); break; // 4
                    case 0x06: piano_play_sound(NOTE_G4); break; // 5
                    case 0x07: piano_play_sound(NOTE_A4); break; // 6
                    case 0x08: piano_play_sound(NOTE_B4); break; // 7
                    case 0x09: piano_play_sound(NOTE_C5); break; // 8
                    default: nosound(); break;
                }
        }
    }
}