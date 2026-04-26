#include "libc/stdint.h"
#include "libc/stddef.h"
#include <multiboot2.h>

#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "kernel/memory.h"
#include "kernel/pit.h"

/* Defined by the linker script — first byte after the kernel binary */
extern uint32_t end;

int kernel_main(void);

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int main(uint32_t magic, struct multiboot_info* mb_info_addr)
{
    (void)magic;
    (void)mb_info_addr;

    terminal_init();
    gdt_init();
    idt_init();
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    return kernel_main();
}
