#include <libc/stddef.h>
#include <libc/stdint.h>
#include "terminal.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "kernel/memory.h"
#include "kernel/pit.h"
#include "kernel/irq_rush.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

static volatile uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

static size_t terminal_row = 0;
static size_t terminal_column = 0;

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gdt_descriptor;

extern void gdt_flush(uint32_t gdt_descriptor_address);
extern uint32_t end;
extern uint32_t cpp_new_test(void);

static uint16_t vga_entry(unsigned char character, uint8_t color) {
    return (uint16_t)character | ((uint16_t)color << 8);
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR_WHITE_ON_BLACK);
        }
    }
}

static void terminal_newline(void) {
    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_newline();
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] =
        vga_entry((unsigned char)c, VGA_COLOR_WHITE_ON_BLACK);

    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_newline();
    }
}

void terminal_write(const char* str) {
    size_t i = 0;

    while (str[i] != '\0') {
        terminal_putchar(str[i]);
        i++;
    }
}

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].base_low = (uint16_t)(base & 0xFFFF);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[index].base_high = (uint8_t)((base >> 24) & 0xFF);

    gdt[index].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[index].granularity = (uint8_t)((limit >> 16) & 0x0F);
    gdt[index].granularity |= (uint8_t)((flags & 0x0F) << 4);
    gdt[index].access = access;
}

static void gdt_init(void) {
    gdt_set_entry(0, 0, 0, 0x00, 0x0);
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

    gdt_descriptor.limit = (uint16_t)(sizeof(gdt) - 1);
    gdt_descriptor.base = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdt_descriptor);
}

void main(uint32_t multiboot_magic, uint32_t multiboot_info_address) {
    (void)multiboot_magic;
    (void)multiboot_info_address;

    terminal_initialize();

    terminal_write("Assignment 6 - IRQ Rush: Kernel Panic");
    terminal_write("\nBooting UiAOS subsystems...");

    gdt_init();
    terminal_write("\nGDT loaded.");

    idt_init();
    terminal_write("\nIDT loaded.");

    irq_install();
    terminal_write("\nIRQ support installed.");

    keyboard_init();
    terminal_write("\nKeyboard IRQ installed.");

    terminal_write("\nInitializing kernel memory...");
    init_kernel_memory(&end);

    terminal_write("\nEnabling paging...");
    init_paging();

    print_memory_layout();

    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);
    (void)some_memory;
    (void)memory2;
    (void)memory3;

    terminal_write("\nTesting overloaded C++ new...");
    if (cpp_new_test() == 42) {
        terminal_write("\nC++ new test passed.");
    } else {
        terminal_write("\nC++ new test FAILED.");
    }

    terminal_write("\nInitializing PIT...");
    init_pit();
    terminal_write("\nPIT initialized.");

    terminal_write("\nStarting IRQ Rush...");
    irq_rush_init();

    terminal_write("\nEnabling hardware interrupts...");
    __asm__ volatile("sti");

    while (1) {
        irq_rush_update();
        irq_rush_render();
        __asm__ volatile("hlt");
    }
}
