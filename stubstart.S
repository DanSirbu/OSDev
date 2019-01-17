global _start
extern main
_start:
    call main

    mov eax, 0 ; sysexit 
    mov ebx, 0 ; sysexit arg
    int 0x80 ; system call number