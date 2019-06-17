global OnSignal
extern SignalReceived

; The kernel call OnSignal whenever there is a signal ready
; We must be careful to not modify any registers

; On the stack we have
; actual PC <- address we must return to after signal finishes
; signum <- signal number


SIGNUM_OFFSET EQU 32 ; pushad pushes 8 registers, 8 * 4 = 32

OnSignal:
    pushad

    ; Push signum parameter
    mov eax, [esp+SIGNUM_OFFSET]
    push eax


    call SignalReceived

    add esp, 4 ; Pop signum we pushed before call

    popad

    add esp, 4 ; Pop signum the kernel pushed

    ret