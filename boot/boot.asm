; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
; When debugging this file, don't forget to substract 0xC0000000 from the address since it's linked there
FLAGS EQU 0x02
bits 32
section .text
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd FLAGS                    ;flags
        dd - (0x1BADB002 + FLAGS)   ;checksum. m+f+c should be zero

global start
global load_idt
global LoadNewPageDirectory, DisablePSE, boot_page_directory

extern kmain 		;this is defined in the c file
extern _kernel_end

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

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti 				;turn on interrupts
	ret

KERN_BASE equ 0xC0000000
KERN_HEAP equ 0xD0000000

KERN_PAGE_NUM equ KERN_BASE >> 22
KERN_HEAP_NUM equ KERN_HEAP >> 22

KERNEL_STACK_SIZE equ 4096

section .data
align 0x1000
boot_page_directory:
	times 4096 dd 0

_kernel_end_num: dd 0

boot_page_directory_phy equ (boot_page_directory - KERN_BASE)
_kernel_end_phy equ (_kernel_end - KERN_BASE)
_kernel_end_num_phy equ (_kernel_end_num - KERN_BASE)

section .text
start:
	;cli 				;block interrupts
	; Carefull not to mess up ebx, it is the multiboot header pointer 

	; Set _kernel_end_num (i.e. the index in boot_page_directory where _kernel_end is)
	mov edi, _kernel_end
	shr edi, 22
	mov [_kernel_end_num_phy], edi

	; Identity mapping
	mov [boot_page_directory_phy], dword 0x00000083

	; esi = index
	; edi = physical address + flags
	mov esi, KERN_PAGE_NUM
	mov edi, 0x00000083
map_kernel_code:
	mov [boot_page_directory_phy + esi * 4], edi

	inc esi
	add edi, 0x00400000 ; Next 4MB

	cmp esi, [_kernel_end_num_phy]
	jl map_kernel_code

	; Map first 4MB of heap to _kernel_end
	mov esi, KERN_HEAP_NUM
	mov edi, _kernel_end_phy
	or edi, 0x83
	mov [boot_page_directory_phy + esi * 4], edi
	 

	; Write boot_page_directory to cpu
	mov ecx, boot_page_directory_phy
	mov cr3, ecx

	mov ecx, cr4
    or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx

	mov ecx, cr0
	or ecx, 1 << 31 ; Enable paging by setting PG Bit 
	mov cr0, ecx
	
	lea ecx, [StartHigherHalf]
	jmp ecx

LoadNewPageDirectory:
	mov ecx, [esp + 4]
	mov cr3, ecx
	ret

DisablePSE:
	mov eax, cr4
 	and eax, 0xFFFFFFEF
 	mov cr4, eax
	ret

StartHigherHalf:
	mov dword [boot_page_directory], 0
	invlpg [0]

	mov esp, kernel_stack_lowest_address + KERNEL_STACK_SIZE

	push dword 0 ; "Previous" eip, so we know to stop our stack trace
	push dword 0 ; "Previous" ebp

	mov ebp, esp

	; Push the multiboot header info
	add ebx, KERN_BASE
	push ebx

	call kmain
	hlt 				;halt the CPU

; since we are using cdecl
; the following registers are assumed to be changed to the called method (i.e. the caller saves them)
; eax, ecx and edx

; we can think of a context switch as if the process calls the cpu_switch_proc method and it returns some time later
; **old_context [ebp+4]
; *new_context [ebp+8]

 ; Remember, c struct is lowest address to highest address
 ; so edi is lowest, eip is highest
global switch_context
 
switch_context:
	mov eax, [esp+4] ; Ptr to Ptr of context, which we will set to this stack
	mov edx, [esp+8] ; Ptr to new context

	; EIP saved from method call
	push ebp
	push ebx
	push esi
	push edi

	; Switch context

	; set old_context pointer to the values saved above (so esp)
	; i.e. *old_context = esp (note the single *)
	mov [eax], esp

	; update stack pointer to new context
	mov esp, edx ; Now, we are on the new stack

	pop edi
	pop esi
	pop ebx
	pop ebp

	ret ; Continue execution
	
global get_ebp
get_ebp:
	mov eax, ebp
	ret

global kernel_stack_lowest_address
section .bss
align 4
kernel_stack_lowest_address:
resb KERNEL_STACK_SIZE; 8KB for stack
