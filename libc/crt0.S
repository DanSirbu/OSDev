global _start
global environ

extern main
extern exit

_start:
    mov eax, [esp+8]
    mov [environ], eax

    call main

    push DWORD eax ; exit code
    call exit

section .data
environ dd 0