extern irq_handler

global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

%macro IRQ_STUB 2
irq%1:
    cli
    push dword 0    ; error code
    push dword %2   ; interrupt number (0x20 + irq)
    jmp irq_common_stub
%endmacro

irq_common_stub:
    pusha

    mov ax, ds      ; save data segment
    push eax
    mov ax, 0x10    ; kernel data segment selector
    mov ds, ax   ; switch to kernel data segment
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp         ; pass pointer to stack frame
    call irq_handler ; call the common IRQ handler
    add esp, 4       ; clean up the stack (remove the argument)

    pop eax          ; restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8       ; clean up the stack (remove error code and interrupt number)
    sti
    iret

IRQ_STUB 0, 0x20
IRQ_STUB 1, 0x21
IRQ_STUB 2, 0x22
IRQ_STUB 3, 0x23
IRQ_STUB 4, 0x24
IRQ_STUB 5, 0x25
IRQ_STUB 6, 0x26
IRQ_STUB 7, 0x27
IRQ_STUB 8, 0x28
IRQ_STUB 9, 0x29
IRQ_STUB 10, 0x2A
IRQ_STUB 11, 0x2B
IRQ_STUB 12, 0x2C
IRQ_STUB 13, 0x2D
IRQ_STUB 14, 0x2E
IRQ_STUB 15, 0x2F