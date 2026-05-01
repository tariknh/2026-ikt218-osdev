extern isr_handler  ; the C function we call from the common stub

global dummy_isr
global isr0
global isr1
global isr2

; ISR that immediatly returns; used to initialise all IDT entries.
dummy_isr: 
	iretd
; ISR 0, Division By Zero, the CPU does not automatically push an error code.
isr0:
    cli
    push dword 0   ; dummy error code
    push dword 0   ; interrupt number
    jmp isr_common_stub

; ISR 1, Debug, the CPU does not automatically push an error code.
isr1:
    cli
    push dword 0   ; dummy error code
    push dword 1   ; interrupt number
    jmp isr_common_stub

; ISR 2, Non-Maskable Interrupt, Debug, the CPU does not automatically push an error code.
isr2:
    cli
    push dword 0   ; dummy error code
    push dword 2   ; interrupt number
    jmp isr_common_stub


; Stub used by alL ISR. Handles pushing CPU registers and datasegment in accordance with the struct registers_t defined in isr.h
; Then handles calling the isr_handler with registers_t argument, then restores the CPU state.
isr_common_stub:
    pusha               ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

	; NOTE: currently our operating system only has a single datasegment, but to
	; allow for compatability with user space in the future we save the current datasegment, load the
	; kernel datasegment, and restore the datasegment after the ISR handle finishes.
    mov ax, ds          ; save the current data segment
    push eax            ; pushes the extended ax register to ensure 32 bit alignment

    mov ax, 0x10        ; load the kernel data segment into every segment register
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Pushes the pointer to the top of the stack to the stack. which represents the register_t struct used by the isr_handler.
    call isr_handler    ; Call the C handler.
    add esp, 4          ; Moves the stack pointer forward with 4 bytes moving past the stack pointer we pushed to the stack.

    pop eax             ; Pops the data segment back off the stack. And restores it back into all segment registers
    mov ds, ax           
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; Restores the general purpose registers pushed onto the stack by `pusha`.
    add esp, 8          ; Move the stack pointer 8 bytes forward to discard the interrupt number and error code we pushed onto the stack. 
    sti
    iret                ; Return from interrupt, handles popping of the elements which the CPU automatically pushed to the stack upon interrupt.


