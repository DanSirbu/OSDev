global _start
global environ

extern main
extern exit

_start:
    ; We are called by the kernel with these arguments: argc, argv, envp
    mov eax, [esp+8]
    mov [environ], eax

    call main

    push DWORD eax ; exit code
    call exit

section .data
environ dd 0