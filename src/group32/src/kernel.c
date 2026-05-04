#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "screen.h"
#include "memory.h"
#include "paging.h"
#include "pit.h"
#include "libc/stdint.h"
#include "song_player.h"
#include "song/song.h"
#include "pong.h"

extern uint32_t end; // Symbol from linker script marking kernel end

// Plays all songs
void play_music(void) {
    // Song list
    Song songs[] = {
        { starwars_theme, sizeof(starwars_theme) / sizeof(Note) },
        { battlefield_1942_theme, sizeof(battlefield_1942_theme) / sizeof(Note) },
        { music_3, sizeof(music_3) / sizeof(Note) },
        { music_4, sizeof(music_4) / sizeof(Note) }
    };

    uint32_t n_songs = sizeof(songs) / sizeof(Song);     // Count songs
    SongPlayer* player = create_song_player();     // Create song player

    // Stop if player creation failed
    if (!player) {
        screen_write("Failed to create SongPlayer\n");
        return;
    }

    // Play each song
    for (uint32_t i = 0; i < n_songs; i++) {
        screen_write("Playing song...\n");
        player->play_song(&songs[i]);
        screen_write("Finished song.\n");
    }

    free(player);     // Free song player memory
}
// Shows system information
void show_system_info(void) {
    screen_clear();

    // Print system status
    screen_write("=== System Information ===\n\n");
    screen_write("GDT: installed\n");
    screen_write("IDT: installed\n");
    screen_write("Interrupts: installed\n");
    screen_write("Keyboard: active\n");
    screen_write("PIT timer: active\n");
    screen_write("PC Speaker: ready\n\n");

    print_memory_layout();     // Print memory layout

    screen_write("\nPress any key to return to menu...");     // Wait before returning

    keyboard_get_char();
}

// Shows the main OS menu
void show_main_menu(void) {
    while (1) {
        screen_clear();
         // Print menu title
        screen_write("========================================\n");
        screen_write("    Welcome to Group32 Operating System\n");
        screen_write("========================================\n\n");
         // Print menu options
        screen_write("1. Music Player\n");
        screen_write("2. Play Pong Game\n");
        screen_write("3. System Information\n");
        screen_write("4. Clear Screen\n");
        screen_write("5. Halt OS\n\n");
        screen_write("Press 1-5: ");

        char key = keyboard_get_char();        // Read user choice

        // Start music player
        if (key == '1') {
            screen_clear();
            screen_write("Starting Music Player...\n\n");

            play_music();

            screen_write("\nMusic finished.\n");
            screen_write("Press any key to return to menu...");
            keyboard_get_char();
        }
       // Start Pong game
        else if (key == '2') {
             play_pong_game();
        }
        // Show system information
        else if (key == '3') {
            show_system_info();
        }
        // Clear screen
        else if (key == '4') {
            screen_clear();
            screen_write("Screen cleared.\n");
            screen_write("Press any key to return to menu...");
            keyboard_get_char();
        }
        // Halt the OS
        else if (key == '5') {
            screen_clear();
            screen_write("Group32 OS halted.\n");

            while (1) {
                __asm__ volatile ("hlt");
            }
        }
    }
}
// Kernel entry point
void kernel_main(void) {
    // Install descriptor tables
    gdt_install();
    screen_clear();

    // Install interrupts
    idt_install();
    isr_install();
    irq_install();
    
    // Initialize memory systems
    init_kernel_memory(&end);
    init_paging();
   
    // Initialize drivers
    keyboard_install();
    init_pit();

    __asm__ volatile ("sti");     // Enable interrupts

    show_main_menu();     // Start main menu

    // Halt if menu returns
    while (1) {
        __asm__ volatile ("hlt");
    }
}