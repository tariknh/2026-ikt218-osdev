[BITS 32]
global idt_load
global outb
global inb

global isr0
global isr1
global isr2

extern isr_handler
extern irq_handler

global enable_interrupts

enable_interrupts:
    sti     ; Set Interrupt Flag
    ret


%macro IRQ 2
  global irq%1
  irq%1:
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; --- Define all 16 IRQs ---
; Syntax: IRQ [IRQ Number] [IDT Vector]
IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

outb:
    mov dx, [esp + 4]   ; Get the port from the stack
    mov al, [esp + 8]   ; Get the value from the stack
    out dx, al          ; Output the value to the port
    ret

inb:
    mov dx, [esp + 4]   ; Get the port from the stack
    xor eax, eax        ; Clear eax
    in al, dx           ; Read from the port into al
    ret                 ; Return value is in eax (al)

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; ISR 0: Divide By Zero Exception
isr0:
    push byte 0      ; Dummy error code
    push byte 0      ; Interrupt number
    jmp isr_common_stub

; ISR 1: Debug Exception
isr1:
    push byte 0
    push byte 1
    jmp isr_common_stub

; ISR 2: Non Maskable Interrupt Exception
isr2:
    push byte 0
    push byte 2
    jmp isr_common_stub

isr_common_stub:
    pusha            ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    mov ax, ds       ; Lower 16-bits of eax = ds
    push eax         ; Save the data segment descriptor

    mov ax, 0x10     ; Load the Kernel Data Segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop eax          ; Reload the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa             ; Pops edi,esi,ebp...
    add esp, 8       ; Cleans up the pushed error code and pushed ISR number
    iret             ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call irq_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret