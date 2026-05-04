BITS 32

EXTERN isr_handler

GLOBAL isr0
GLOBAL isr1
GLOBAL isr2

isr0:
    cli
    push dword 0
    call isr_handler
    add esp, 4
    iret

isr1:
    cli
    push dword 1
    call isr_handler
    add esp, 4
    iret

isr2:
    cli
    push dword 2
    call isr_handler
    add esp, 4
    iret