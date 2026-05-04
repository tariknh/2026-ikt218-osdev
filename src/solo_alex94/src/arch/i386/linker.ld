ENTRY(_start)

SECTIONS {
    . = 1M;

    /* Place the multiboot header where the bootloader can find it */
    .boot :
    {
        *(.multiboot_header)
    }

    /* Executable kernel code */
    . = ALIGN(4K);
    .text :
    {
        *(.text .text.*)
    }

    /* Read-only data such as string literals */
    . = ALIGN(4K);
    .rodata :
    {
        *(.rodata .rodata.*)
    }

    /* Writable initialized data */
    . = ALIGN(4K);
    .data :
    {
        *(.data .data.*)
    }

    /* Uninitialized data */
    . = ALIGN(4K);
    .bss :
    {
        *(.bss .bss.*)
    }

    end = .;
    _end = .;
    __end = .;
}
