extern IsrHandler
extern IrqHandler

isr_common_stub:
    ; 1. Save CPU state
    pusha           ; Pushes general purpose registers onto stack
	mov ax, ds      ; Lower 16-bits of eax = ds.
	push eax        ; save the data segment descriptor
	mov ax, 0x10    ; loads data segment descriptor into kernel
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
    ; Pass the current stack frame to C so IsrHandler can read one uniform
    ; Registers layout regardless of which interrupt vector fired.
    push esp        ; pushes stack pointer as arg to function

    call IsrHandler       

    ; 2. Restore CPU state
    add esp, 4      ; removes arg pushed to function
	pop eax         ; restores the data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa            ; restores general purpose registers
	add esp, 8      ; removes interrupt num and error num from stack
	iret            ; returns from interrupt

irq_common_stub:
    ; 1. Save CPU state
    pusha           ; Pushes general purpose registers onto stack
	mov ax, ds      ; Lower 16-bits of eax = ds.
	push eax        ; save the data segment descriptor
	mov ax, 0x10    ; loads data segment descriptor into kernel
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
    push esp        ; pushes stack pointer as arg to function

    call IrqHandler

    ; 2. Restore CPU state
    add esp, 4      ; removes arg pushed to function
	pop eax         ; restores the data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa            ; restores general purpose registers
	add esp, 8      ; removes interrupt num and error num from stack
	iret            ; returns from interrupt
    
%macro isr_no_err_stub 1
isr_stub_%+%1:
    ; Some CPU exceptions do not push an error code, so add a dummy one
    ; to keep the C-visible stack layout identical for every ISR.
    push dword 0         ; pushes dummy error
    push dword %1        ; pushes interrupt vector number
    jmp isr_common_stub
%endmacro

%macro isr_err_stub 1
isr_stub_%+%1:
    ; Exceptions in this group already pushed an error code before entering
    ; the stub, so only the interrupt vector itself needs to be added here.
    push dword %1        ; pushes interrupt vector number
    jmp isr_common_stub
%endmacro

%macro irq_stub 1
irq_stub_%+%1:
    ; Hardware IRQs never arrive with a CPU error code, and PIC-remapped IRQs
    ; live at vectors 32-47 rather than the CPU exception range 0-31.
    push dword 0         ; pushes dummy error
    push dword (32 + %1) ; pushes interrupt vector number + 32 to account for isr's
    jmp irq_common_stub
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_stub 0
irq_stub 1
irq_stub 2
irq_stub 3
irq_stub 4
irq_stub 5
irq_stub 6
irq_stub 7
irq_stub 8
irq_stub 9
irq_stub 10
irq_stub 11
irq_stub 12
irq_stub 13
irq_stub 14
irq_stub 15

global isr_stub_table
isr_stub_table:
%assign i 0           ; creates a variable and sets it to 0
%rep    32 
    dd isr_stub_%+i   ; adds one address to the table
%assign i i+1         ; increments variable
%endrep

global irq_stub_table
irq_stub_table:
%assign i 0           ; creates a variable and sets it to 0
%rep    16
    dd irq_stub_%+i   ; adds one address to the table
%assign i i+1         ; increments variable
%endrep
