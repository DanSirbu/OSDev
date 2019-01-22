global _start
extern main
extern exit
_start:
    call main

    push DWORD eax ; exit code
    call exit