extern irq_handler ; C function that handles IRQ

global irq0 ; Defines entrypoints to the different IRQs
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

; Stub used by alL IRQ. Handles pushing CPU registers and datasegment in accordance with the struct registers_t defined in isr.h
; Then handles calling the isr_handler with registers_t argument, then restores the CPU state.
irq0:  cli
    push dword 0
    push dword 32
    jmp irq_common_stub

irq1:  cli
    push dword 0
    push dword 33
    jmp irq_common_stub

irq2:  cli
    push dword 0
    push dword 34
    jmp irq_common_stub

irq3:  cli
    push dword 0
    push dword 35
    jmp irq_common_stub

irq4:  cli
    push dword 0
    push dword 36
    jmp irq_common_stub

irq5:  cli
    push dword 0
    push dword 37
    jmp irq_common_stub

irq6:  cli
    push dword 0
    push dword 38
    jmp irq_common_stub

irq7:  cli
    push dword 0
    push dword 39
    jmp irq_common_stub

irq8:  cli
    push dword 0
    push dword 40
    jmp irq_common_stub

irq9:  cli
    push dword 0
    push dword 41
    jmp irq_common_stub

irq10: cli
    push dword 0
    push dword 42
    jmp irq_common_stub

irq11: cli
    push dword 0
    push dword 43
    jmp irq_common_stub

irq12: cli
    push dword 0
    push dword 44
    jmp irq_common_stub

irq13: cli
    push dword 0
    push dword 45
    jmp irq_common_stub

irq14: cli
    push dword 0
    push dword 46
    jmp irq_common_stub

irq15: cli
    push dword 0
    push dword 47
    jmp irq_common_stub

; See isr_common_stub in isr.asm for comments as this stub follows the exact same principles the only difference being calling irq_handler instead of isr_handler
irq_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10        ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pointer to registers_t
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8          ; remove int_no and err_code
    sti
    iret
