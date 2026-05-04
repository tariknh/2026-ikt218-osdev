#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include "libc/stdio.h"
#include <multiboot2.h>

#include "vga.h"
#include "gdt.h"
#include "isr.h"
#include "irq.h"
#include "kernel_memory.h"
#include "pit.h"
#include "libc/string.h"


#include "song.h"
#include "frequencies.h"
#include "piano.h"

extern uint32_t end;


struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int kernel_main(void);

enum {
    SCREEN_PAUSE_MS = 4000
};

static void wait_for_user_next_screen(void) {
    uint32_t start_events = irq_get_keyboard_event_count();
    printf("\nPress any key to continue...\n");
    while (irq_get_keyboard_event_count() == start_events) {
        asm volatile("sti; hlt");
    }
}

static void halt_forever(void) {
    while (true) {
        asm volatile("hlt");
    }
}


int compute(int a, int b) { 
    return a + b;
}

// Example to make data from assembly to C 
typedef struct{
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e[6];
} MyStruct; 




// assignment 2
void assignment_2() {
    clear_text(0x0F);
    reset_cursor();
    printf("Hello World");
}

// assignment 3 (usikker på om jeg har fått denne riktig)
void assignment_3() {
    wait_for_user_next_screen();
    asm volatile("int $0x3");
    wait_for_user_next_screen();
    asm volatile("int $0x4");
    wait_for_user_next_screen();
    asm volatile("int $0x0"); 
    wait_for_user_next_screen();
}

// assignment 4
void assignment_4() {
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();
    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);
    printf("malloc #1: %p\n", some_memory);
    printf("malloc #2: %p\n", memory2);
    printf("malloc #3: %p\n", memory3);
    free(some_memory);
    free(memory2);
    free(memory3);
    // Ingen annen opprydding nødvendig
}

// assignment 5
void assignment_5() {
    printf("Playing music_1...\n");
    Song* song = malloc(sizeof(Song));
    song->notes = music_1;
    song->length = sizeof(music_1) / sizeof(Note);
    play_song(song);
    free(song);
}

void userMenu() {
    bool user_exit = false;
    while (!user_exit) {
        clear_text(0x0F);
        clear_text(0x0F);
        int choice = 0;


        printf("   ___       _    ___               ___  ___ \n");
        printf("  / _ \\ _  _(_)__|   \\ __ _ _  _   / _ \\/ __|\n");
        printf(" | (_) | || | / _| |) / _` | || | | (_) \\__ \\\n");
        printf("  \\__\\_\\_,_|_\\__|___/\\__,_|\\_, |  \\___/|___/\n");
        printf("              --QuicDay OS  |__/              \n");
        printf("\n");
        printf("\n");
        printf("Select an option:\n");
        printf("1. Booting and Printing\n");
        printf("2. Interrupts\n");
        printf("3. Memory and PIT\n");    
        printf("4. Music Player\n");
        printf("5. Piano\n");
        printf("6. Exit\n");

        while (1) {
            if (inb(0x64) & 0x01) {
                uint8_t scancode = inb(0x60);
                if (scancode >= 0x02 && scancode <= 0x07) { // 1 til 6
                    choice = scancode - 0x01; // 1 til 6
                    break;
                }
            }
        }

        // needs return functionality
        switch (choice) {
            case 1:
                assignment_2();
                break;
            case 2:
                assignment_3();
                break;
            case 3:
                assignment_4();
                break;
            case 4:
                assignment_5();
                break;
            case 5:
                printf("     QuicDay Piano!     \n");
                printf("Play by pressing 1-8 on the keypad\n");
                printf("Press 9 to return to menu\n\n");
                piano_play_sound_keys();
                //clear_text(0x0F);
                break;
            case 6:
                // Exit message without functionality..
                printf("byeee!\n");
                user_exit = true;
                halt_forever();
                break;
            default:
                printf("Invalid choice, please try again.\n");
                break;
        }
        // At the end of each action, clear the screen and loop back to the menu
        sleep_interrupt(500);
        wait_for_user_next_screen();
        sleep_interrupt(500);
        clear_text(0x0F);
        clear_text(0x0F);
    }
}

int main(uint32_t myStruct, uint32_t magic, struct multiboot_info* mb_info_addr) {
   
    initGdt();
    isr_initialize();
    irq_initialize();
    init_kernel_memory(&end);
    init_paging();
    init_pit();
    asm volatile("sti");
   
    userMenu();
    return 0;
}

