[extern isr_handler]
[extern irq_handler]
[extern syscall_handler]

isr_common_stub:
	pusha			; Pushes edi, esi, ebp, esp, ebx, edx, edx, eax

	mov ax, ds		; Lower 16-bits of eax = ds.
	push eax		; save the data segment descriptor

	mov ax, 0x10	; load the kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

    push esp
    cld
	call isr_handler
	add esp, 4

	pop eax			; reload the original data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa			; Pops edi, esi, ebp...
	add esp, 8		; Cleans up the pushed error code and pushed ISR number
	sti
	iret			; pops 5 things at once, CS, EIP, EFLAGS, SS, and ESP


irq_common_stub:
	pusha			; Pushes edi, esi, ebp, esp, ebx, edx, edx, eax

	mov ax, ds		; Lower 16-bits of eax = ds.
	push eax		; save the data segment descriptor

	mov ax, 0x10	; load the kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

    push esp
    cld
	call irq_handler
	add esp, 4

	pop eax			; reload the original data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa			; Pops edi, esi, ebp...
	add esp, 8		; Cleans up the pushed error code and pushed ISR number
	sti
	iret			; pops 5 things at once, CS, EIP, EFLAGS, SS, and ESP


; defines all CPU defined ISRs
%macro isr_err_stub 1
isr_stub_%+%1:
    CLI
    PUSH BYTE %1
    JMP isr_common_stub
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    CLI
    PUSH BYTE 0
    PUSH BYTE %1
    JMP isr_common_stub
%endmacro

%macro irq 2
irq_stub_%+%1:
    CLI
    PUSH BYTE 0
    PUSH BYTE %2
    JMP irq_common_stub
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

irq  0,  32
irq  1,  33
irq  2,  34
irq  3,  35
irq  4,  36
irq  5,  37
irq  6,  38
irq  7,  39
irq  8,  40
irq  9,  41
irq 10,  42
irq 11,  43
irq 12,  44
irq 13,  45
irq 14,  46
irq 15,  47

global isr_stub_table
isr_stub_table:
    %assign i 0
    %rep    32
        DD isr_stub_%+i
    %assign i i+1
    %endrep

global irq_stub_table
irq_stub_table:
    %assign i 0
    %rep    16
        DD irq_stub_%+i
    %assign i i+1
    %endrep

global syscall_stub
syscall_stub:
    CLI
    PUSH 0
    PUSH 0x80
    PUSHA

    mov ebx, esp
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push ebx
    cld
    call syscall_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    POPA
    add esp, 8
    iret
