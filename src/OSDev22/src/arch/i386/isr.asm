; isr.asm - Assembly-stubber for CPU-exceptions (interrupt 0-31)
;
; Noen interrupts pusher en error code automatisk (f.eks. #8, #10-14),
; mens andre gjør det ikke. For de som ikke gjør det, pusher vi en
; dummy verdi (0) så stacken alltid ser lik ut for C-handleren.

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

extern isr_handler

section .text
bits 32

; ---- Stubber uten error code (pusher dummy 0) ----

isr0:
    push 0
    push 0
    jmp isr_common

isr1:
    push 0
    push 1
    jmp isr_common

isr2:
    push 0
    push 2
    jmp isr_common

isr3:
    push 0
    push 3
    jmp isr_common

isr4:
    push 0
    push 4
    jmp isr_common

isr5:
    push 0
    push 5
    jmp isr_common

isr6:
    push 0
    push 6
    jmp isr_common

isr7:
    push 0
    push 7
    jmp isr_common

isr8:
    push 8
    jmp isr_common

isr9:
    push 0
    push 9
    jmp isr_common

isr10:
    push 10
    jmp isr_common

isr11:
    push 11
    jmp isr_common

isr12:
    push 12
    jmp isr_common

isr13:
    push 13
    jmp isr_common

isr14:
    push 14
    jmp isr_common

isr15:
    push 0
    push 15
    jmp isr_common

isr16:
    push 0
    push 16
    jmp isr_common

isr17:
    push 17
    jmp isr_common

isr18:
    push 0
    push 18
    jmp isr_common

isr19:
    push 0
    push 19
    jmp isr_common

isr20:
    push 0
    push 20
    jmp isr_common

isr21:
    push 21
    jmp isr_common

isr22:
    push 0
    push 22
    jmp isr_common

isr23:
    push 0
    push 23
    jmp isr_common

isr24:
    push 0
    push 24
    jmp isr_common

isr25:
    push 0
    push 25
    jmp isr_common

isr26:
    push 0
    push 26
    jmp isr_common

isr27:
    push 0
    push 27
    jmp isr_common

isr28:
    push 0
    push 28
    jmp isr_common

isr29:
    push 29
    jmp isr_common

isr30:
    push 30
    jmp isr_common

isr31:
    push 0
    push 31
    jmp isr_common

; ---- Felles kode for alle ISR-er ----

isr_common:
    pusha               ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    mov ax, ds
    push eax            ; Lagre DS på stacken (matcher isr_frame.ds)

    mov ax, 0x10        ; Kernel data-segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Send peker til stackframen som argument
    call isr_handler
    add esp, 4          ; Fjern argumentet fra stacken

    pop ebx             ; Gjenopprett DS
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa                ; Gjenopprett alle registre

    add esp, 8          ; Fjern int_no og err_code fra stacken
    iret                ; Returner fra interrupt