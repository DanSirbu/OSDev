BITS 32
EXTERN interrupt_handler

%macro no_error 1
idt_%1:
    push 0
    push %1
    jmp common_interrupt_handler
%endmacro
%macro error 1
idt_%1:
    push %1
    nop
    nop
    jmp common_interrupt_handler
%endmacro

common_interrupt_handler:
    pusha
    mov eax, cr2
    push eax
    ;call interrupt_handler
    add esp, 4 ; ignore cr2 
    popa
    add esp, 8 ; pop handler number and error code
    iretd

no_error 0