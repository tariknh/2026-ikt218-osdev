extern irq_handler

; Common IRQ stub. Saves state, calls C handler, restores state.
irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    popa
    add esp, 8
    iret

; Macro to generate IRQ stubs. IRQs do not push error codes.
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0          ; Dummy error code
    push byte %2         ; Interrupt number (mapped IDT entry)
    jmp irq_common_stub
%endmacro

; Define IRQ stubs for IRQ 0-15, mapped to IDT 32-47
IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47
