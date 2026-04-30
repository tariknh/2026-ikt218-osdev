#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <libc/stdint.h>
#include <libc/stdio.h>
#include <memory.h>
#include <keyboard.h>
#include <pit.h>
#include <song.h>
#include <input.h>
#include <menu.h>


#define VGA_ADDRESS  0xB8000
#define VGA_COLS     80
#define VGA_ROWS     25

#define VGA_COLOR    0x0F

extern uint32_t end;





static volatile unsigned short *vga = (volatile unsigned short *)VGA_ADDRESS;


static void serial_init(void)
{
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static void serial_write_char(char c)
{
    outb(0x3F8, (unsigned char)c);
}

static void serial_write(const char *str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n')
            serial_write_char('\r');
        serial_write_char(str[i]);
    }
}

static int terminal_col = 0;
static int terminal_row = 0;

void terminal_clear(void)
{
    int row;
    int col;

    for (row = 0; row < VGA_ROWS; row++)
        for (col = 0; col < VGA_COLS; col++)
            vga[row * VGA_COLS + col] = (unsigned short)(' ' | (VGA_COLOR << 8));

    terminal_col = 0;
    terminal_row = 0;
}

void terminal_scroll(void)
{
    int row;
    int col;

    for (row = 1; row < VGA_ROWS; row++)
        for (col = 0; col < VGA_COLS; col++)
            vga[(row - 1) * VGA_COLS + col] = vga[row * VGA_COLS + col];

    for (col = 0; col < VGA_COLS; col++)
        vga[(VGA_ROWS - 1) * VGA_COLS + col] =
            (unsigned short)(' ' | (VGA_COLOR << 8));

    terminal_row = VGA_ROWS - 1;
}

void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_col = 0;
    } else if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_col = VGA_COLS - 1;
        }
    } else {
        vga[terminal_row * VGA_COLS + terminal_col] =
            (unsigned short)((unsigned char)c | (VGA_COLOR << 8));
        terminal_col++;

        if (terminal_col >= VGA_COLS) {
            terminal_col = 0;
            terminal_row++;
        }
    }

    if (terminal_row >= VGA_ROWS)
        terminal_scroll();
}

void terminal_write(const char *str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}

void main(void)
{
    serial_init();
    serial_write("kernel: entered main\n");

    gdt_init();
    serial_write("kernel: gdt loaded\n");

    pic_remap();

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);

    idt_init();
    serial_write("kernel: idt loaded\n");

    init_kernel_memory(&end);
    init_paging();

    init_pit();

    input_init();

    terminal_clear();

    __asm__ volatile("sti");

    printf("lazarOS booting...\n");
    printf("GDT loaded, IDT loaded, PIT running at %d Hz\n", TARGET_FREQUENCY);
    printf("Memory and paging initialised.\n\n");
    sleep_interrupt(1500);

    menu_run();

    while (1)
        __asm__ volatile("hlt");
}