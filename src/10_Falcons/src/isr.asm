extern isr_handler

global isr0
global isr1
global isr2

isr0:
    cli
    push dword 0
    call isr_handler
    add esp, 4
    sti
    iret

isr1:
    cli
    push dword 1
    call isr_handler
    add esp, 4
    sti
    iret

isr2:
    cli
    push dword 2
    call isr_handler
    add esp, 4
    sti
    iret