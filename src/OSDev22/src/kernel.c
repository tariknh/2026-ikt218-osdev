#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "libc/stdint.h"
#include "gdt.h"
#include "terminal.h"

/*
 * Minimal multiboot2 info header passed to us by the bootloader.
 * The full structure is defined in multiboot2.h; we only need the
 * size and the pointer to the first tag here.
 */
struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

/*
 * main - kernel entry point
 *
 * Called from multiboot2.asm after the stack is set up.
 * Arguments are pushed by the assembly stub:
 *   magic       - must equal MULTIBOOT2_BOOTLOADER_MAGIC (0x36d76289)
 *   mb_info_addr - pointer to the multiboot2 information structure
 */
int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    /* Set up the Global Descriptor Table (flat 4 GB code + data segments) */
    gdt_init();

    /* Initialise the VGA text-mode terminal and clear the screen */
    terminal_initialize();

    /* --- Hello World --- */
    terminal_writestring("Hello World\n");

    /* Show that printf works too */
    //printf("Multiboot2 magic: 0x%x\n", magic);

    return 0;
}
