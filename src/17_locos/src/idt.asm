; Name: idt.asm
; Projet: LocOS
; Description: Assembly code for interrupt handler entry points and the lidt instruction.
; Each of ISR and IRQ stub saves registers and passes the vector number to C, and restores the state.

; Export the loader and all interrupt stubs
global idt_load
global isr0
global isr1
global isr2
global isr48
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
global isr_ignore

extern isr_handler_c
extern irq_handler_c

; Load the IDT pointer into the CPU
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

; Empty handler for unused vectors
isr_ignore:
    iretd

; CPU exception stubs
isr0:
    push dword 0
    jmp isr_common

isr1:
    push dword 1
    jmp isr_common

isr2:
    push dword 2
    jmp isr_common

isr48:
    push dword 0x30
    jmp isr_common

; IRQ stubs after PIC remap
irq0:
    push dword 0
    jmp irq_common
irq1:
    push dword 1
    jmp irq_common
irq2:
    push dword 2
    jmp irq_common
irq3:
    push dword 3
    jmp irq_common
irq4:
    push dword 4
    jmp irq_common
irq5:
    push dword 5
    jmp irq_common
irq6:
    push dword 6
    jmp irq_common
irq7:
    push dword 7
    jmp irq_common
irq8:
    push dword 8
    jmp irq_common
irq9:
    push dword 9
    jmp irq_common
irq10:
    push dword 10
    jmp irq_common
irq11:
    push dword 11
    jmp irq_common
irq12:
    push dword 12
    jmp irq_common
irq13:
    push dword 13
    jmp irq_common
irq14:
    push dword 14
    jmp irq_common
irq15:
    push dword 15
    jmp irq_common

; Common ISR path that calls the C handler
isr_common:
    pusha
    ; After pusha, the pushed vector number is at [esp+32].
    push dword [esp+32]
    call isr_handler_c
    add esp, 4
    popa
    add esp, 4
    iretd

; Common IRQ path that calls the C handler
irq_common:
    pusha
    ; Same stack layout as isr_common: IRQ number is at [esp+32].
    push dword [esp+32]
    call irq_handler_c
    add esp, 4
    popa
    add esp, 4
    iretd
