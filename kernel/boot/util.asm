global LoadNewPageDirectory, DisablePSE, tss_flush, load_idt, asm_lgdt,get_ebp, get_flags, switch_context, update_segment_registers

section .text
; asm_lgdt - load global descriptor table
; stack: [esp + 4] the address of the gdt description structure
;        [esp    ] return address
asm_lgdt:
  mov edx, [esp + 4]
  lgdt [edx]
  ret

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
	ret

LoadNewPageDirectory:
	mov ecx, [esp + 4]
	mov cr3, ecx
	ret

DisablePSE:
	mov eax, cr4
 	and eax, 0xFFFFFFEF
 	mov cr4, eax
	ret


update_segment_registers:
	mov ax, 0x10 ; kernel data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	jmp 0x8:return_func ; Far jump to change cs register

return_func:
	ret


; since we are using cdecl
; the following registers are assumed to be changed to the called method (i.e. the caller saves them)
; eax, ecx and edx

; we can think of a context switch as if the process calls the cpu_switch_proc method and it returns some time later
; **old_context [ebp+4]
; *new_context [ebp+8]

 ; Remember, c struct is lowest address to highest address
 ; so edi is lowest, eip is highest

;EAX, ECX, and EDX are caller saved
switch_context:
	pop eax ; Eip is restored after context switch

	mov ecx, [esp] ; Ptr to old context
	mov edx, [esp+4] ; Ptr to new context

	; Save registers in old context, if context_t changes, this must be changed
	mov [ecx], eax ; eip
	mov [ecx+4], esp
	mov [ecx+8], ebp
	mov [ecx+12], ebx
	mov [ecx+16], esi
	mov [ecx+20], edi

	; Restore new context registers
	mov ebp, [edx+8]
	mov ebx, [edx+12]
	mov esi, [edx+16]
	mov edi, [edx+20]

	; Change stack
	mov esp, [edx+4]
	push dword [edx] ; push eip
	ret ; Continue execution from eip

switch_context_old:
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
	
get_ebp:
	mov eax, ebp
	ret

get_flags:
	pushf
	pop eax
	ret