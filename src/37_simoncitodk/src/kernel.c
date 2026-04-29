#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pic.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdint.h>

extern uint32_t end;
extern uint32_t *test_new_operator(void);

void kernel_main(void)
{
    gdt_init();
    idt_init();
    pic_remap();
    pic_mask_all_except_timer_and_keyboard();

    init_kernel_memory(&end);
    init_paging();

    void *some_memory = malloc(12345);
    void *memory2 = malloc(54321);
    void *memory3 = malloc(13331);

    free(memory2);
    void *memory4 = malloc(1000);

    uint32_t *new_value = test_new_operator();

    init_pit();

    terminal_write("Memory manager test\n");

    terminal_write("malloc 1: ");
    terminal_write_hex((uint32_t)some_memory);
    terminal_write("\n");

    terminal_write("malloc 2: ");
    terminal_write_hex((uint32_t)memory2);
    terminal_write("\n");

    terminal_write("malloc 3: ");
    terminal_write_hex((uint32_t)memory3);
    terminal_write("\n");

    terminal_write("reuse after free: ");
    terminal_write_hex((uint32_t)memory4);
    terminal_write("\n");

    terminal_write("new uint32_t: ");
    terminal_write_hex((uint32_t)new_value);
    terminal_write(" = ");
    terminal_write_dec(*new_value);
    terminal_write("\n");

    print_memory_layout();

    terminal_write("Busy sleep start\n");
    asm volatile("sti");
    sleep_busy(2000);
    terminal_write("Busy sleep done\n");

    terminal_write("Interrupt sleep start\n");
    sleep_interrupt(2000);
    terminal_write("Interrupt sleep done\n");

    terminal_write("Press keys to trigger IRQ1\n");

    for (;;) {
        asm volatile("hlt");
    }
}
