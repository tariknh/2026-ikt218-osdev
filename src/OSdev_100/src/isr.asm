; isr.asm
global isr_stub_table
extern exception_handler

; Build a stub for exceptions where the CPU does not push an error code.
; We push a dummy 0 so every interrupt reaches isr_common with the same layout.
%macro isr_no_err 1
isr_stub_%+%1:
    cli
    push byte 0    ; push dummy error code
    push byte %1   ; push interrupt number
    jmp isr_common
%endmacro

; Build a stub for exceptions where the CPU already pushed an error code.
; In this case we only need to push the interrupt number ourselves.
%macro isr_err 1
isr_stub_%+%1:
    cli
    push byte %1   ; push interrupt number
    jmp isr_common
%endmacro


; Build a stub for a hardware IRQ after PIC remapping.
; IRQs do not push an error code, so we again add a dummy 0.
%macro irq_stub 2
global isr_stub_%+%2
isr_stub_%+%2:
    cli
    push byte 0
    push byte %2        ; Push the remapped interrupt vector (32, 33, ...)
    jmp isr_common
%endmacro



; Shared interrupt path for all exceptions and IRQs.
; By the time execution reaches here, the stub has already pushed:
;   1. an error code (real or dummy)
;   2. the interrupt number
isr_common:
    ; Save all general-purpose registers so we can restore the interrupted code later.
    pusha
    
    ; Save the current data segment, then switch to the kernel data segment
    ; so the C handler can safely access kernel memory.
    mov ax, ds
    push eax

    mov ax, 0x10    ; 0x10 = kernel data segment selector from the GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Hand the current interrupt stack frame to the C handler.
    push esp
    call exception_handler
    add esp, 4

    ; Restore the original data segment values.
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore the saved general-purpose registers.
    popa

    ; Remove the interrupt number and error code from the stack.
    add esp, 8

    ; Return from the interrupt and resume the interrupted code.
    iret


; All 32 exception handlers
isr_no_err 0   ; divide by zero
isr_no_err 1   ; debug
isr_no_err 2   ; non maskable interrupt
isr_no_err 3   ; breakpoint
isr_no_err 4   ; overflow
isr_no_err 5   ; bound range exceeded
isr_no_err 6   ; invalid opcode
isr_no_err 7   ; device not available
isr_err    8   ; double fault
isr_no_err 9   ; coprocessor segment overrun
isr_err    10  ; invalid TSS
isr_err    11  ; segment not present
isr_err    12  ; stack segment fault
isr_err    13  ; general protection fault
isr_err    14  ; page fault
isr_no_err 15  ; reserved
isr_no_err 16  ; x87 floating point
isr_err    17  ; alignment check
isr_no_err 18  ; machine check
isr_no_err 19  ; SIMD floating point
isr_no_err 20  ; virtualization
isr_no_err 21  ; reserved
isr_no_err 22  ; reserved
isr_no_err 23  ; reserved
isr_no_err 24  ; reserved
isr_no_err 25  ; reserved
isr_no_err 26  ; reserved
isr_no_err 27  ; reserved
isr_no_err 28  ; reserved
isr_no_err 29  ; reserved
isr_err    30  ; security exception
isr_no_err 31  ; reserved

; Define the 16 remapped hardware IRQ stubs.
; After PIC remapping, IRQ0-IRQ15 become vectors 32-47.
irq_stub 0, 32  ; Timer
irq_stub 1, 33  ; Keyboard (This is the one you want!)
irq_stub 2, 34
irq_stub 3, 35
irq_stub 4, 36
irq_stub 5, 37
irq_stub 6, 38
irq_stub 7, 39
irq_stub 8, 40
irq_stub 9, 41
irq_stub 10, 42
irq_stub 11, 43
irq_stub 12, 44
irq_stub 13, 45
irq_stub 14, 46
irq_stub 15, 47

; Table of stub addresses used by idt.c when filling the IDT.
isr_stub_table:
%assign i 0
%rep 48
    dd isr_stub_%+i
%assign i i+1
%endrep
