global _start
extern main
extern syscall_exit
_start:
    call main

    jmp syscall_exit