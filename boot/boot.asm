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
	;mov dword [page_directory], 0
	;invlpg [0]

	mov esp, stack_space
	mov ebp, esp
	
	call kmain
	hlt 				;halt the CPU

section .bss
resb 8192; 8KB for stack
stack_space:
