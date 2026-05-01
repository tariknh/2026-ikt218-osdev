extern kernel_main

global _start

section .multiboot_header
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

align 8
    dw 0
    dw 0
    dd 8
header_end:

section .text
bits 32

_start:
    cli

    mov esp, stack_top

    push ebx
    push eax

    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 4096 * 16
stack_top:
