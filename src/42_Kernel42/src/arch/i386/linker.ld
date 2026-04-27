ENTRY(_start)              /* bootloader entry point symbol */

SECTIONS {
    . = 1M;                /* bootloader loads kernel at 0x100000 */

    /* Multiboot header (must be first 8KB) */
    .boot :
    {
        *(.multiboot_header)
    }

    /* Page Directory (CR3 target, 4KB aligned) */
    .page_dir 0x00104000 :
    {
        _page_dir = .;     /* C symbol: uint32_t* page_directory = 0x104000 */
        . = . + 4096;      /* Reserve 1 page (1024 PDEs) */
    }

    /* Page Tables (identity-map kernel 0x100000-0x200000) */
    .page_tables 0x00108000 :
    {
        _page_tables = .;  /* C: First PT covers kernel text */
        . = . + 8192;      /* 2 pages = 1024 PTEs (2MB identity map) */
    }

    /* Standard ELF sections (after fixed tables) */
    .text :
    {
        *(.text .text.*)   /* Kernel code */
    }

    .rodata :
    {
        *(.rodata .rodata.*) /* Const data */
    }

    .data :
    {
        *(.data .data.*)   /* Initialized data */
    }

    .bss :
    {
        *(.bss .bss.*)     /* Zero-init data */
    }

    end = .; _end = .; __end = .;  /* Heap/brk end symbols */
}