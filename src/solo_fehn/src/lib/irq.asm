; irq.asm - 16 IRQ entry points (vectors 32..47)
;
; IRQ stubs are like ISR stubs but always push a dummy error code, since
; hardware IRQs never push one.  The vector number is the IDT slot we
; registered the stub in (32..47), not the raw IRQ number (0..15) - that
; way irq_handler can look up which IRQ it was by computing int_no - 32.

%macro IRQ 2
global irq%1
irq%1:
    cli
    push    dword 0           ; dummy error code
    push    dword %2          ; vector number (32 + IRQ index)
    jmp     irq_common
%endmacro

extern irq_handler

section .text
bits 32

IRQ 0,  32      ; PIT timer
IRQ 1,  33      ; PS/2 keyboard
IRQ 2,  34      ; cascade (slave PIC)
IRQ 3,  35      ; COM2 / COM4
IRQ 4,  36      ; COM1 / COM3
IRQ 5,  37      ; LPT2 / sound card
IRQ 6,  38      ; floppy disk
IRQ 7,  39      ; LPT1 / spurious
IRQ 8,  40      ; CMOS real-time clock
IRQ 9,  41      ; free
IRQ 10, 42      ; free
IRQ 11, 43      ; free
IRQ 12, 44      ; PS/2 mouse
IRQ 13, 45      ; FPU / coprocessor
IRQ 14, 46      ; primary ATA
IRQ 15, 47      ; secondary ATA

irq_common:
    pusha

    mov     ax, ds
    push    eax

    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    push    esp
    call    irq_handler
    add     esp, 4

    pop     eax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    popa
    add     esp, 8
    iret
