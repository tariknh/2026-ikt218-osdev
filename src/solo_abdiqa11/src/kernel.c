#include "idt.h"
#include <stdint.h>
#include "isr.h"
#include "terminal.h"
#include "kernel/memory.h"
#include "libc/stdio.h"
#include "irq.h"
#include "kernel/pit.h"
#include "games/snake.h"

extern uint32_t end;

#define VGA_MEMORY (uint16_t*)0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

typedef unsigned long size_t;

static uint16_t* terminal_buffer = VGA_MEMORY;
static uint8_t terminal_color = 0x0F; // White on black
static uint32_t terminal_row = 0;
static uint32_t terminal_column = 0;

static void terminal_putentryat(char c, uint8_t color, uint32_t x, uint32_t y) {
    const uint32_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = ((uint16_t)color << 8) | c;
}

static void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void main(void) {
    /* IDT + software interrupt setup */
    idt_init();
    terminal_write("IDT limit should be 2047\n");
    isr_install();

    /* IRQ + PIT setup */
    irq_install();
    init_pit();

    /* Memory manager + paging */
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    /* malloc verification */
    void* a = malloc(12345);
    void* b = malloc(54321);
    void* c = malloc(13331);
    printf("malloc a=%x b=%x c=%x\n", (uint32_t)a, (uint32_t)b, (uint32_t)c);

    terminal_write("Hello World\n");

    /* Software interrupt tests */
    asm volatile("int $0x03");
    asm volatile("int $0x04");
    asm volatile("int $0x05");

    /* Enable hardware interrupts */
    asm volatile("sti");

    /* Start Assignment 6 feature: Snake */
    snake_init();

    while (1) {
        snake_draw();
        sleep_interrupt(200);
        snake_update();
    }
}