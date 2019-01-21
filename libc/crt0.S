global _start
extern main
extern exit
_start:
    call main

    jmp exit