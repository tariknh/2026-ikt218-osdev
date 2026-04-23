#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "memory.h"
#include "paging.h"
#include "pit.h"
#include "songs/song.h"

// This symbol is exported by arch/i386/linker.ld
extern uint32_t end;
volatile int quit = 0;

/*------------------ GDT ------------------*/

struct gdt_entry{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr _gp;

extern void _gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran){
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}      

void gdt_install(){
    _gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    _gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    _gdt_flush();
}
		
/*--------------------- PRINT ----------------------*/	

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}



void terminal_putchar(char c) 
{
    // Check new lines
    if (c == '\n') {
        terminal_column = 0;
        ++terminal_row;

        if (terminal_row == VGA_HEIGHT) {// Check if we need to scroll
            for (size_t y = 1; y < VGA_HEIGHT; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                    const size_t src = y * VGA_WIDTH + x;
                    const size_t dst = (y - 1) * VGA_WIDTH + x;
                    terminal_buffer[dst] = terminal_buffer[src];
                }
            }

            for (size_t x = 0; x < VGA_WIDTH; x++) {
                terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
            }

            terminal_row = VGA_HEIGHT - 1;
        }

        return; // Return since we dont want to print the new line character
    }

	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
	terminal_write(data, strlen(data));
}

//--------------------------------------------------------------------------------//

// Simple printf supporting %s, %d, %u, %x
void printf(const char* fmt, ...) {
    // Minimal varargs implementation
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    char num_buf[12];

    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            terminal_putchar(*p);
            continue;
        }
        p++;
        if (*p == 'd' || *p == 'i') {
            int val = __builtin_va_arg(args, int);
            if (val < 0) { terminal_putchar('-'); val = -val; }
            int i = 0;
            if (val == 0) { num_buf[i++] = '0'; }
            else { while (val > 0) { num_buf[i++] = '0' + (val % 10); val /= 10; } }
            for (int j = i - 1; j >= 0; j--) terminal_putchar(num_buf[j]);
        } else if (*p == 'u') {
            unsigned int val = __builtin_va_arg(args, unsigned int);
            int i = 0;
            if (val == 0) { num_buf[i++] = '0'; }
            else { while (val > 0) { num_buf[i++] = '0' + (val % 10); val /= 10; } }
            for (int j = i - 1; j >= 0; j--) terminal_putchar(num_buf[j]);
        } else if (*p == 's') {
            const char* s = __builtin_va_arg(args, const char*);
            terminal_writestring(s);
        } else if (*p == 'x') {
            unsigned int val = __builtin_va_arg(args, unsigned int);
            terminal_writestring("0x");
            int i = 0;
            if (val == 0) { num_buf[i++] = '0'; }
            else { while (val > 0) { int n = val & 0xF; num_buf[i++] = (n < 10) ? '0' + n : 'A' + n - 10; val >>= 4; } }
            for (int j = i - 1; j >= 0; j--) terminal_putchar(num_buf[j]);
        } else {
            terminal_putchar('%');
            terminal_putchar(*p);
        }
    }

    __builtin_va_end(args);
}


/*----------music-----------*/
void play_music() {
    // How to play music
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)}
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    SongPlayer* player = create_song_player();

    while(!quit) {
        for(uint32_t i = 0; i < n_songs; i++) {
            if (quit) break;
            printf("Playing Song...\n");
            player->play_song(&songs[i]);
            printf("Finished playing the song.\n");
        }
    }
}


/*------- MENU -------*/
void show_menu(void) {
    terminal_writestring("\n"); 
    terminal_writestring("  +===============================+\n");
    terminal_writestring("  |      UiAOS Main Menu          |\n");
    terminal_writestring("  +===============================+  What do you \n");
    terminal_writestring("  | 1. Play Music                 |  want to do? \n");
    terminal_writestring("  |                               |      ___\n");
    terminal_writestring("  |                               |    (o v o)\n");
    terminal_writestring("  | 4. Memory Viewer              |      /|\\\n");
    terminal_writestring("  +===============================+      / \\\n");
    terminal_writestring("\n  Press a key to select...\n");
}

/*--------- interrupt test --------*/
void interrupt_test(counter){
    while (!quit) {
                printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
                sleep_busy(1000);
                if (quit) break;
                printf("[%d]: Slept using busy-waiting.\n", counter++);

                printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
                sleep_interrupt(1000);
                if (quit) break;
                printf("[%d]: Slept using interrupts.\n", counter++);
            }
}

void main(){
    // Initialize the monitor (screen output)
    terminal_initialize();

    // Initialize the Global Descriptor Table (GDT)
    gdt_install();

    // Initialize the Interrupt Descriptor Table (IDT)
    idt_install();

    // Initialize ISR exception handlers
    isrs_install();

    // Initialize hardware interrupts (IRQs)
    irq_install();

    // Initialize keyboard
    keyboard_install();

    // Initialize the kernel's memory manager using the end address of the kernel
    init_kernel_memory(&end); // <------ THIS IS PART OF THE ASSIGNMENT

    // Initialize paging for memory management
    init_paging(); // <------ THIS IS PART OF THE ASSIGNMENT

    // Print memory information
    print_memory_layout(); // <------ THIS IS PART OF THE ASSIGNMENT

    // Initialize PIT
    init_pit(); // <------ THIS IS PART OF THE ASSIGNMENT

    terminal_writestring("\n ");
	terminal_writestring("           \xB2\xDB\xB2\n");
	terminal_writestring("          \xB2\xDB\xDB\xDB\xB2\n");
	terminal_writestring("         \xB2\xDB\xDB\xB0\xDB\xDB\xB2\n");
	terminal_writestring("        \xB2\xDB\xDB\xB0 \xB0\xDB\xDB\xB2\n");
	terminal_writestring("\xB2\xB2\xB2\xB2\xDB\xDB\xDB\xDB\xDB\xB0     \xB0\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xDB\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\xB0\n");
	terminal_writestring(" \xB0\xDB\xDB\xB0\n");
	terminal_writestring("   \xDB\xDB\xB2                \xB2\xDB\xDB\xDB\xDB\xB2     \xB2\xDB\xDB\xDB\xB2\n");
	terminal_writestring("     \xDB\xDB\xB2              \xDB\xB0   \xB0\xDB   \xDB\xB0\n");
	terminal_writestring("     \xB2\xDB\xB0              \xDB     \xDB   \xDB\xB2\n");
	terminal_writestring("    \xDB\xDB\xB0    \xB2          \xDB     \xDB    \xB0\xB0\xB0\xB0\xDB\xB2\n");
	terminal_writestring("  \xB2\xDB\xB0    \xB2\xDB\xB0\xDB\xB2        \xDB\xB2   \xB2\xDB         \xDB\n");
	terminal_writestring(" \xDB\xDB  \xB2\xB2\xDB\xDB\xB0  \xB0\xDB\xB2       \xB0\xDB\xDB\xDB\xDB\xDB\xB0   \xB0\xDB\xB2\xB2\xB2\xDB\xB0\n");
	terminal_writestring("  \xB0\xDB\xDB\xB0\xB0       \xB0\xDB\xB2\n");
	terminal_writestring("                \xB0\xB0\n");
	terminal_writestring(" ----------------------------------------------- \n");
    

    // Test malloc as the teacher's example shows
    void* some_memory = malloc(12345);
    void* memory2     = malloc(54321);
    void* memory3     = malloc(13331);
    (void)some_memory; (void)memory2; (void)memory3;



	/* Enable hardware interrupts */
	asm volatile("sti");

    // Evaluation loop — demonstrates both sleep modes
    int counter = 0;


    printf("\n Do you want to see the menu? \n (reach it whenever you want by writing menue/Menu in terminal)): ");
    while(1) {
        
        char answer = keyboard_getchar();
        if (answer == 'm' || answer == 'M') {
            char answer = keyboard_getchar();
            if(answer == 'e' || answer == 'E'){
                char answer = keyboard_getchar();
                if(answer == 'n' || answer == 'N'){
                    char answer = keyboard_getchar();
                    printf("\n\n");
                    if(answer == 'u' || answer == 'U'){
                        char answer = keyboard_getchar();
                        show_menu();
                    } 
                } 
            } 
        }
        else if (answer == '1'){
            quit = 0;
            //PLAY MUSIC
            play_music();
            printf("\n");
        } 
        else if (answer == '2'){
            quit = 0;
            interrupt_test(counter);
            printf("\n");
        }
    }
    
    for(;;);
}