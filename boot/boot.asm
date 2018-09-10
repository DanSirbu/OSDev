; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html

bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd 0x00                    ;flags
        dd - (0x1BADB002 + 0x00)   ;checksum. m+f+c should be zero

global start
global keyboard_handler
global read_port
global write_port
global load_idt

extern kmain 		;this is defined in the c file
extern keyboard_handler_main

global asm_lgdt

; asm_lgdt - load global descriptor table
; stack: [esp + 4] the address of the gdt description structure
;        [esp    ] return address
asm_lgdt:
  mov edx, [esp + 4]
  lgdt [edx]
  ret

GLOBAL tss_flush   ; Allows our C code to call tss_flush().
tss_flush:
  mov ax, 0x2B      ; Load the index of our TSS structure - The index is
                   ; 0x28, as it is the 5th selector and each is 8 bytes
                   ; long, but we set the bottom two bits (making 0x2B)
                   ; so that it has an RPL of 3, not zero.
  ltr ax            ; Load 0x2B into the task state register.
  ret

read_port:
	mov edx, [esp + 4]
			;al is the lower 8 bits of eax
	in al, dx	;dx is the lower 16 bits of edx
	ret

write_port:
	mov   edx, [esp + 4]    
	mov   al, [esp + 4 + 4]  
	out   dx, al  
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti 				;turn on interrupts
	ret

keyboard_handler:                 
	call    keyboard_handler_main
	iretd


KERN_BASE equ 0xC0000000
KERN_PAGE_NUM equ KERN_BASE >> 22
KERNEL_STACK_SIZE equ 4096

section .data
align 0x1000
global page_directory
page_directory:
	dd 0x00000083 ; Set Present, set 4mb pages, set rw
	times (KERN_PAGE_NUM - 1) dd 0
	dd 0x00000083 ; Set Present, set 4mb pages, set rw
	times (1024 - KERN_PAGE_NUM - 1) dd 0

section .text
start:
	;cli 				;block interrupts
	; Write page_directory to cpu
	mov ecx, (page_directory - KERN_BASE)
	mov cr3, ecx

	mov ecx, cr4
    or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx

	mov ecx, cr0
	or ecx, 1 << 31 ; Enable paging by setting PG Bit 
	mov cr0, ecx
	
	lea ecx, [StartHigherHalf]
	jmp ecx

StartHigherHalf:
	mov dword [page_directory], 0
	invlpg [0]

	mov esp, kernel_stack_lowest_address + KERNEL_STACK_SIZE
	mov ebp, esp
	
	call kmain
	hlt 				;halt the CPU

global kernel_stack_lowest_address
section .bss
align 4
kernel_stack_lowest_address:
resb KERNEL_STACK_SIZE; 8KB for stack
