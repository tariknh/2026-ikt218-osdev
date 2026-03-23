extern main
global _start
global gdt_flush  ; makes gdt_flush visible to C code
global idt_flush  ; makes idt_flush visible to C code

section .multiboot_header
header_start:
    dd 0xe85250d6 	                                                ; Magic number (multiboot 2)
    dd 0				                                            ; Architecture 0 (protected mode i386)
    dd header_end - header_start 	                                ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum

;align 8
;framebuffer_tag_start:
;    dw 5                                              ; type
;    dw 1                                              ; flags
;    dd framebuffer_tag_end - framebuffer_tag_start    ; size
;    dd 800                                            ; width
;    dd 600                                            ; height
;    dd 32                                             ; depth
;framebuffer_tag_end:

align 8
    ; Required end tag:
    dw 0	; type
    dw 0	; flags
    dw 8	; size
header_end:

section .text
bits 32

_start:
    cli

    mov esp, stack_top

	push ebx
	push eax

    call main ; Jump main function

; hlt loop to avoid the Operating System immediately exiting
.hang: 
    hlt ; hlt instruction makes CPU wait for an interrupt before continuing
    jmp .hang

; Function is called from C with (uint32_t)&gdt_pointer as argument
gdt_flush:
	mov eax, [esp+4] ; Moves the GDT pointer from the stack to eax register
					; esp+4 is used to go past the return memory address 
					; pushed onto the stack and [] dereferences the 
					; value at that memory address
 
	lgdt [eax] ; Loads the GDT pointer into the CPU.

	mov ax, 0x10 ; Datasegment is at the GDT entry 0x10, and is loaded into the ax register
	mov ds, ax ; loads the address of the data segment GDT entry into all segment registers.
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:.flush ; Uses a jmp instruction to change the code segment to GDT entry 0x08,
					; .flush immediately returns.
.flush:
	ret


; idt_flush(uint32_t idt_ptr_address)
; Called from C with the address of our idt_ptr_t struct.
; The lidt instruction reads limit+base from that address and loads the IDT.
idt_flush:
    mov eax, [esp+4]   ; first argument: address of our idt_ptr_t
    lidt [eax]         ; load the IDT (same idea as lgdt for the GDT)
    ret

section .bss
stack_bottom:
    resb 4096 * 16
stack_top:
