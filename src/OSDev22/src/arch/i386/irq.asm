; irq.asm - Assembly-stubber for hardware-interrupts (IRQ 0-15)
;
; Nesten identiske med ISR-stubbene, men bruker interrupt-nummer
; 32-47 (etter PIC-remapping). Ingen har error codes.

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

extern irq_handler

section .text
bits 32

irq0:
    push 0
    push 32
    jmp irq_common

irq1:
    push 0
    push 33
    jmp irq_common

irq2:
    push 0
    push 34
    jmp irq_common

irq3:
    push 0
    push 35
    jmp irq_common

irq4:
    push 0
    push 36
    jmp irq_common

irq5:
    push 0
    push 37
    jmp irq_common

irq6:
    push 0
    push 38
    jmp irq_common

irq7:
    push 0
    push 39
    jmp irq_common

irq8:
    push 0
    push 40
    jmp irq_common

irq9:
    push 0
    push 41
    jmp irq_common

irq10:
    push 0
    push 42
    jmp irq_common

irq11:
    push 0
    push 43
    jmp irq_common

irq12:
    push 0
    push 44
    jmp irq_common

irq13:
    push 0
    push 45
    jmp irq_common

irq14:
    push 0
    push 46
    jmp irq_common

irq15:
    push 0
    push 47
    jmp irq_common

; ---- Felles kode for alle IRQ-er ----

irq_common:
    pusha               ; Lagre alle generelle registre

    mov ax, ds
    push eax            ; Lagre DS på stacken

    mov ax, 0x10        ; Last kernel data-segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Send stackframe-peker som argument
    call irq_handler
    add esp, 4          ; Rydd opp argumentet

    pop ebx             ; Gjenopprett DS
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa                ; Gjenopprett registre

    add esp, 8          ; Fjern int_no og err_code
    iret                ; Tilbake fra interrupt