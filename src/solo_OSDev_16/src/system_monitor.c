#include "system_monitor.h"
#include "terminal.h"
#include "pit.h"
#include "memory.h"
#include "song/song.h"
#include "irq.h"
#include "keyboard.h"

#include <libc/stdint.h>

#define MAX_ALLOCATIONS 16
#define ALLOCATION_SIZE 256

// Stores pointers returned malloc() from memory monitor
static void* allocation_ptr[MAX_ALLOCATIONS];

// Tracks how many memory monitor allocations are currently active.
static uint32_t allocation_count = 0;

// Stores which monitor page is currently active.
static int current_monitor_view = 1;

// Stores which song should be played by the kernel loop.
static uint32_t selected_song = 0;

// Stores which timer test was last completed.
static uint32_t last_timer_test = 0;

// Timer ticks during last timer test
static uint32_t last_timer_test_ticks = 0;

// Stores which timer test should be executed by the kernel loop.
static uint32_t pending_timer_test = 0;


// Reused controlers for every screen.
void system_monitor_print_controls() {
    terminal_write("\nViews:\n");
    terminal_write("[1] Overview\n");
    terminal_write("[2] Memory\n");
    terminal_write("[3] Timer / Interrupts\n");
    terminal_write("[4] Paging\n");
    terminal_write("[5] PC Speaker\n");
}

// Paging monitor screen
void system_monitor_paging_screen() {
    terminal_clear();

    terminal_write("========================================\n");
    terminal_write("           PAGING MONITOR\n");
    terminal_write("========================================\n\n");

    terminal_write("Paging Status\n");
    terminal_write("-------------\n");

    terminal_write("Enabled: ");
    terminal_write(paging_is_enabled() ? "Yes" : "No");
    terminal_write(" | Page size: ");
    print_dec(paging_get_page_size());
    terminal_write(" bytes\n");

    terminal_write("Mapped: ");
    print_hex(paging_get_mapped_start());
    terminal_write(" - ");
    print_hex(paging_get_mapped_end());
    terminal_write("\n\n");

    terminal_write("CPU Paging Registers\n");
    terminal_write("--------------------\n");

    terminal_write("CR0 PG bit: ");
    terminal_write(paging_is_enabled() ? "Enabled" : "Disabled");
    terminal_write("\n");

    terminal_write("CR3 directory base: ");
    print_hex(paging_get_cr3());
    terminal_write("\n\n");

    terminal_write("Page Structures\n");
    terminal_write("---------------\n");

    terminal_write("Directory: ");
    print_hex(paging_get_page_directory_address());
    terminal_write("\n");

    terminal_write("First table: ");
    print_hex(paging_get_first_page_table_address());
    terminal_write(" | Last table: ");
    print_hex(paging_get_last_page_table_address());
    terminal_write("\n\n");

    terminal_write("Mapping\n");
    terminal_write("-------\n");

    terminal_write("Identity mapping: Enabled\n");

    terminal_write("Kernel virtual: ");
    print_hex(memory_get_kernel_start());
    terminal_write(" | physical: ");
    print_hex(memory_get_kernel_start());
    terminal_write("\n\n");

    system_monitor_print_controls();
}

void system_monitor_interrupts_screen() {
    terminal_clear();

    terminal_write("========================================\n");
    terminal_write("       TIMER / INTERRUPT MONITOR\n");
    terminal_write("========================================\n\n");

    terminal_write("PIT / Timer\n");
    terminal_write("-----------\n");

    terminal_write("Timer ticks since boot: ");
    print_dec(get_current_tick());
    terminal_write("\n");

    terminal_write("PIT frequency: ");
    print_dec(get_pit_frequency());
    terminal_write(" Hz\n");

    terminal_write("System uptime: ");
    print_dec(get_current_tick() / get_pit_frequency());
    terminal_write(" seconds\n\n");

    terminal_write("Keyboard / IRQ1\n");
    terminal_write("---------------\n");

    terminal_write("Keyboard interrupt events: ");
    print_dec(get_irq1_count());
    terminal_write("\n");

    terminal_write("Last keyboard scancode and key: ");
    print_hex(get_last_keyboard_scancode());
    terminal_write(" / ");
    terminal_putchar(get_last_keyboard_key());
    terminal_write("\n\n");

    terminal_write("Timer Tests\n");
    terminal_write("-----------\n");
    terminal_write("B - Busy sleep test\n");
    terminal_write("H - HLT sleep test\n");
    terminal_write("R - Reset timer test result\n");

    terminal_write("\nLast timer test\n");
    terminal_write("---------------\n");

    if (last_timer_test == 0) {
        terminal_write("None\n");
    } else if (last_timer_test == 1) {
        terminal_write("Busy sleep completed\n");
        terminal_write("Elapsed ticks: ");
        print_dec(last_timer_test_ticks);
        terminal_write("\n");
    } else if (last_timer_test == 2) {
        terminal_write("HLT sleep completed\n");
        terminal_write("Elapsed ticks: ");
        print_dec(last_timer_test_ticks);
        terminal_write("\n");
    }

    system_monitor_print_controls();
}

void system_monitor_pc_speaker_screen() {
    terminal_clear();

    terminal_write("========================================\n");
    terminal_write("          PC SPEAKER MONITOR\n");
    terminal_write("========================================\n\n");

    terminal_write("Audio Output\n");
    terminal_write("------------\n");

    terminal_write("Choose the song you want to play\n");

    terminal_write("Available songs\n");
    terminal_write("---------------\n");
    terminal_write("Q - Play Super Mario\n");
    terminal_write("W - The Legend of Zelda\n");
    terminal_write("E - Ode to Joy (Beethoven)\n");
    terminal_write("T - Fader Jakob  4\n");
    terminal_write("Y - Megalovania\n");
    terminal_write("U - Imperial March (Star Wars)\n\n");

    system_monitor_print_controls();
}

void system_monitor_memory_screen() {
    terminal_clear();

    terminal_write("========================================\n");
    terminal_write("          MEMORY MONITOR\n");
    terminal_write("========================================\n\n");

    terminal_write("Kernel Memory\n");
    terminal_write("-------------\n");

    terminal_write("Kernel start: ");
    print_hex(memory_get_kernel_start());
    terminal_write("\n");

    terminal_write("Kernel end: ");
    print_hex(memory_get_kernel_end());
    terminal_write("\n");

    terminal_write("Kernel size: ");
    print_dec(memory_get_kernel_size());
    terminal_write(" bytes\n\n");

    terminal_write("Heap Memory\n");
    terminal_write("-----------\n");

    terminal_write("Heap start: ");
    print_hex(memory_get_heap_start());
    terminal_write("\n");

    terminal_write("Heap current: ");
    print_hex(memory_get_heap_current());
    terminal_write("\n");

    terminal_write("Heap end: ");
    print_hex(memory_get_heap_end());
    terminal_write("\n");

    terminal_write("Heap used: ");
    print_dec(memory_get_heap_used());
    terminal_write(" bytes\n");

    terminal_write("Heap remaining: ");
    print_dec(memory_get_heap_remaining());
    terminal_write(" bytes\n");

    terminal_write("Active allocations: ");
    print_dec(allocation_count);
    terminal_write("\n\n");

    terminal_write("\nAllocating memory\n");
    terminal_write("---------------\n");
    terminal_write("A - Allocate memory\n");
    terminal_write("F - Free last allocated block\n");

    system_monitor_print_controls();
}

void system_monitor_overview_screen() {
    terminal_clear();

    terminal_write("========================================\n"); 
    terminal_write("        KERNEL SYSTEM MONITOR\n");
    terminal_write("========================================\n\n");

    terminal_write("System Overview\n");
    terminal_write("----------------\n\n");

   terminal_write("Uptime: ");
    print_dec(get_current_tick());
    terminal_write(" ticks / ");
    print_dec(get_current_tick() / get_pit_frequency());
    terminal_write(" seconds\n");

    terminal_write("Last key: ");
    print_hex(get_last_keyboard_scancode());
    terminal_write(" / ");
    terminal_putchar(get_last_keyboard_key());
    terminal_write("\n");

    terminal_write("Keyboard events: ");
    print_dec(get_irq1_count());
    terminal_write("\n");

    terminal_write("Heap: ");
    print_dec(memory_get_heap_used());
    terminal_write(" used / ");
    print_dec(memory_get_heap_remaining());
    terminal_write(" remaining\n");

    terminal_write("Paging: ");
    terminal_write(paging_is_enabled() ? "Enabled" : "Disabled");
    terminal_write("\n");

    terminal_write("Mapped memory: ");
    print_hex(paging_get_mapped_start());
    terminal_write(" - ");
    print_hex(paging_get_mapped_end());
    terminal_write("\n");

    system_monitor_print_controls();
}

void system_monitor_screen() {
    switch (current_monitor_view)
    {
    case 1:
        system_monitor_overview_screen();
        break;

    case 2:
        system_monitor_memory_screen();
        break;

    case 3:
        system_monitor_interrupts_screen();
        break;

    case 4:
        system_monitor_paging_screen();
        break;

    case 5:
        system_monitor_pc_speaker_screen();
        break;
    }   
}

void system_monitor_update() {
    if (pending_timer_test != 0) {
        uint32_t test_to_run = pending_timer_test;
        pending_timer_test = 0;

        uint32_t start_ticks = get_current_tick();

        if (test_to_run == 1) {
            sleep_busy(1000);
            last_timer_test = 1;
        } else if (test_to_run == 2) {
            sleep_interrupt(1000);
            last_timer_test = 2;
        }

        uint32_t end_ticks = get_current_tick();
        last_timer_test_ticks = end_ticks - start_ticks;

        system_monitor_screen();
        return;
    }

    if (selected_song == 0) {
        return;
    }

    uint32_t song_to_play = selected_song;
    selected_song = 0;

    switch (song_to_play)
    {
    case 1:
        play_song(&music_1);
        break;
    case 2:
        play_song(&music_2);
        break;
    case 3:
        play_song(&music_3);
        break;
    case 4:
        play_song(&music_4);
        break;
    case 5:
        play_song(&music_5);
        break;
    case 6:
        play_song(&music_6);
        break;
    }
}

void system_monitor_handle_input(uint8_t scancode) {
    switch (scancode)
    {
    case 0x02: // key 1
        current_monitor_view = 1;
        break;

    case 0x03: // key 2
        current_monitor_view = 2;
        break;

    case 0x04: // key 3
        current_monitor_view = 3;
        break;

    case 0x05: // key 4
        current_monitor_view = 4;
        break;

    case 0x06: // key 5
        current_monitor_view = 5;
        break;
    
    case 0x13: // R key
        if (current_monitor_view == 3) {
            last_timer_test = 0;
            last_timer_test_ticks = 0;
        }
        break;

    case 0x10: // Q key
        if (current_monitor_view == 5) {
            selected_song = 1;
        }
        break;

    case 0x11: // W key
        if (current_monitor_view == 5) {
            selected_song = 2;
        }
        break;

    case 0x12: // E key
        if (current_monitor_view == 5) {
            selected_song = 3;
        }
        break;

    case 0x14: // T key
        if (current_monitor_view == 5) {
            selected_song = 4;
        }
        break;

    case 0x15: // Y key
        if (current_monitor_view == 5) {
            selected_song = 5;
        }
        break;

    case 0x16: // U key
        if (current_monitor_view == 5) {
            selected_song = 6;
        }
        break;

    case 0x1E: // A key
        if (current_monitor_view == 2) {
            if (allocation_count >= MAX_ALLOCATIONS) {
                terminal_write("Max allocations reached, please free memory.\n");
            } else {
                allocation_ptr[allocation_count] = malloc(ALLOCATION_SIZE);
                allocation_count++;
            }
        }
        break;

    case 0x21: // F key
        if (current_monitor_view == 2) {
            if (allocation_count == 0) {
                terminal_write("No allocated memory to free.\n");
            } else {
                allocation_count--;
                free(allocation_ptr[allocation_count]);
                allocation_ptr[allocation_count] = 0;
            }
        }
        break;

    case 0x30: // B key
        if (current_monitor_view == 3) {
            pending_timer_test = 1;
        }
        break;

    case 0x23: // H key
        if (current_monitor_view == 3) {
            pending_timer_test = 2;
        }
        break;

    default:
        return;
    }

    system_monitor_screen();
}