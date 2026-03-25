#include "gdt.h"
#include "terminal.h"
#include "idt.h"

volatile uint32_t tick = 0;

void timer_callback(struct registers *regs) {
    
    tick++;
    static int second_counter = 0;
    second_counter++;

    if (second_counter >= 18) {
        // Update Uptime
        second_counter = 0;
    }
    
}
void sleep(uint32_t ticks_to_wait) {
    uint32_t end_tick = tick + ticks_to_wait;
    while (tick < end_tick) {
        // hlt stops the CPU until the next interrupt (timer or keyboard)
        // This saves power and prevents the CPU from melting in a loop
        __asm__("hlt"); 
    }
}

void main() {

    init_gdt();
    idt_init();
    terminal_initialize();

    irq_install_handler(0, timer_callback);
    irq_install_handler(1, keyboard_handler);
    __asm__ __volatile__("sti");

    
    
    sleep(10);
    __asm__ __volatile__("int $0");
    sleep(10);
    __asm__ __volatile__("int $1");
    sleep(10);
    __asm__ __volatile__("int $2");
    sleep(10);
    terminal_initialize();


    

    
    terminal_write("Hello William\n");

    
    terminal_write("I am the best at coding. Look at me write something:\n");


    while(1) {
        __asm__("hlt");
    }
}

