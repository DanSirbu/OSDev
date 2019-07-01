ALIGN_MOD   EQU	1 << 0 ; align modules to page boundary
MEMINFO 	  EQU 1 << 1 ; provide memory map
VIDINFO 	  EQU 1 << 2 ; provide video info (ex. address of framebuffer)

FLAGS EQU ALIGN_MOD | MEMINFO | VIDINFO

global start

extern kmain 		;this is defined in the c file
extern _kernel_end

KERNEL_STACK_SIZE equ 4096
KERN_BASE equ 0xC0000000

KERN_BASE_INDEX equ KERN_BASE >> 22
boot_page_directory_phy equ (boot_page_directory - KERN_BASE)
KERNEL_END_PHY equ (_kernel_end - KERN_BASE)
KERNEL_END_PHY_INDEX dd 0 ; EQU (KERNEL_END_PHY >> 22)
KERNEL_END_INDEX dd 0 ; EQU (_kernel_end >> 22)


bits 32
section .boot_header
        ;multiboot spec
        align 4
        dd 0x1BADB002              ;magic
        dd FLAGS                    ;flags
        dd -(0x1BADB002 + FLAGS)   ;checksum. m+f+c should be zero
				times 5 dd 0
				dd 0 ; 0 = set graphics mode
			  dd 640, 480, 32 ; Width, height, depth

section .boot_text
start:
	;cli 				;block interrupts
	; Careful not to mess up ebx, it is the multiboot header pointer 

	; Set up constants
	;;;;;;;;;;;;;;;;;;;;;;
	mov eax, KERNEL_END_PHY
	shr eax, 22
	mov [KERNEL_END_PHY_INDEX], eax

	mov eax, _kernel_end
	shr eax, 22
	mov [KERNEL_END_INDEX], eax
	;;;;;;;;;;;;;;;;;;;;;;


	; map 0->kernel_end_phy to 0->kernel_end_phy
	mov esi, 0 ; index in boot_page_directory
	mov edi, 0x00000083  ; physical address + flags entry
map_identity:
	mov [boot_page_directory_phy + esi * 4], edi

	inc esi ; next index
	add edi, 0x00400000 ; Next 4MB

	cmp esi, KERNEL_END_PHY_INDEX
	jl map_identity

	; map 0->kernel_end_phy to KERN_BASE -> KERN_BASE + kernel_end_phy
	mov esi, KERN_BASE_INDEX ; index in boot_page_directory
	mov edi, 0x00000083 ; physical address + flags entry
map_higher_half:
	mov [boot_page_directory_phy + esi * 4], edi

	inc esi
	add edi, 0x00400000 ; Next 4MB

	cmp esi, KERNEL_END_INDEX
	jl map_higher_half
	 

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

StartHigherHalf:
	; mov dword [boot_page_directory], 0
	; invlpg [0]

	mov esp, kernel_stack_lowest_address + KERNEL_STACK_SIZE

	push dword 0 ; "Previous" eip, so we know to stop our stack trace
	push dword 0 ; "Previous" ebp

	mov ebp, esp

	; Push the multiboot header info
	add ebx, KERN_BASE
	push ebx

	call kmain
	hlt 				;halt the CPU


section .data
align 0x1000

global boot_page_directory
boot_page_directory:
	times 4096 dd 0

global kernel_stack_lowest_address
section .bss
align 4
kernel_stack_lowest_address:
resb KERNEL_STACK_SIZE
