#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "libc/stdint.h"
#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "memory.h"
#include "pit.h"
#include "song/song.h"
#include "marlen_sabina_intro.h"
#include "menu.h"

extern uint32_t end;

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    /* Core hardware init */
    gdt_init();
    terminal_initialize();

    idt_init();
    isr_init();
    irq_init();

    init_kernel_memory(&end);
    init_paging();

    init_pit();
    keyboard_init();

    /* Boot splash (pink animated intro) */
    marlen_sabina_intro();

    /* Main menu (pink queen UI) */
    run_menu();

    /* If run_menu() ever returns (Exit option), halt cleanly */
    for (;;) {
        asm volatile ("hlt");
    }

    return 0;
}