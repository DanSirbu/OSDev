BITS 32
EXTERN interrupt_handler
global idt_0

%macro no_error 1
idt_%1:
    push DWORD 0
    push DWORD %1
    jmp common_interrupt_handler
%endmacro
%macro error 1
idt_%1:
    push DWORD %1
    jmp DWORD common_interrupt_handler
%endmacro

common_interrupt_handler:
    pushad

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10 ; kernel data segment, the cs has already been changed by the cpu
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call interrupt_handler

interrupt_return:
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore general registers
    popad

    ; Pop handler number and error code
    add esp, 8

    iretd

global interrupt_return
global enter_userspace ; (entry_point, stack)
enter_userspace:
    mov ebp, esp

    ; bits 0 and 1 are requested priviledge level RPL
    ; bit 2 is the Table Indicator (Ti), Ti = 1 -> get segment from local descriptor table
    ; the rest of the bits are the index
    ; USER_CODE_SEGMENT_INDEX = 3, so index = 3 << 3 = 0x18 
    ; USER_DATA_SEGMENT_INDEX = 4, so index = 4 << 3 = 0x20
    ; 0x18 + 3 = 0x1B
    ; 0x20 + 3 = 0x20
    
    mov ax,0x23
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    ; eip, cs, eflags, useresp, ss
    push DWORD 0x23 ; ss
    push DWORD [ebp+8] ; useresp
    ; pushfd ; flags

    ; Push flags and enable interrupt bit
    pushf
    pop eax
    or eax,0x200 ; Interrupt bit
    push eax

    push DWORD 0x1B ; cs
    push DWORD [ebp+4] ; eip 
    iretd

align 0x10
no_error 0
align 16
no_error 1
align 16
no_error 2
align 16
no_error 3
align 16
no_error 4
align 16
no_error 5
align 16
no_error 6
align 16
no_error 7
align 16
error 8
align 16
no_error 9
align 16
error 10
align 16
error 11
align 16
error 12
align 16
error 13
align 16
error 14
align 16
no_error 15
align 16
no_error 16
align 16
error 17
align 16
no_error 18
align 16
no_error 19
align 16
no_error 20
align 16
no_error 21
align 16
no_error 22
align 16
no_error 23
align 16
no_error 24
align 16
no_error 25
align 16
no_error 26
align 16
no_error 27
align 16
no_error 28
align 16
no_error 29
align 16
error 30
align 16
no_error 31
align 16
no_error 32
align 16
no_error 33
align 16
no_error 34
align 16
no_error 35
align 16
no_error 36
align 16
no_error 37
align 16
no_error 38
align 16
no_error 39
align 16
no_error 40
align 16
no_error 41
align 16
no_error 42
align 16
no_error 43
align 16
no_error 44
align 16
no_error 45
align 16
no_error 46
align 16
no_error 47
align 16
no_error 48
align 16
no_error 49
align 16
no_error 50
align 16
no_error 51
align 16
no_error 52
align 16
no_error 53
align 16
no_error 54
align 16
no_error 55
align 16
no_error 56
align 16
no_error 57
align 16
no_error 58
align 16
no_error 59
align 16
no_error 60
align 16
no_error 61
align 16
no_error 62
align 16
no_error 63
align 16
no_error 64
align 16
no_error 65
align 16
no_error 66
align 16
no_error 67
align 16
no_error 68
align 16
no_error 69
align 16
no_error 70
align 16
no_error 71
align 16
no_error 72
align 16
no_error 73
align 16
no_error 74
align 16
no_error 75
align 16
no_error 76
align 16
no_error 77
align 16
no_error 78
align 16
no_error 79
align 16
no_error 80
align 16
no_error 81
align 16
no_error 82
align 16
no_error 83
align 16
no_error 84
align 16
no_error 85
align 16
no_error 86
align 16
no_error 87
align 16
no_error 88
align 16
no_error 89
align 16
no_error 90
align 16
no_error 91
align 16
no_error 92
align 16
no_error 93
align 16
no_error 94
align 16
no_error 95
align 16
no_error 96
align 16
no_error 97
align 16
no_error 98
align 16
no_error 99
align 16
no_error 100
align 16
no_error 101
align 16
no_error 102
align 16
no_error 103
align 16
no_error 104
align 16
no_error 105
align 16
no_error 106
align 16
no_error 107
align 16
no_error 108
align 16
no_error 109
align 16
no_error 110
align 16
no_error 111
align 16
no_error 112
align 16
no_error 113
align 16
no_error 114
align 16
no_error 115
align 16
no_error 116
align 16
no_error 117
align 16
no_error 118
align 16
no_error 119
align 16
no_error 120
align 16
no_error 121
align 16
no_error 122
align 16
no_error 123
align 16
no_error 124
align 16
no_error 125
align 16
no_error 126
align 16
no_error 127
align 16
no_error 128
align 16
no_error 129
align 16
no_error 130
align 16
no_error 131
align 16
no_error 132
align 16
no_error 133
align 16
no_error 134
align 16
no_error 135
align 16
no_error 136
align 16
no_error 137
align 16
no_error 138
align 16
no_error 139
align 16
no_error 140
align 16
no_error 141
align 16
no_error 142
align 16
no_error 143
align 16
no_error 144
align 16
no_error 145
align 16
no_error 146
align 16
no_error 147
align 16
no_error 148
align 16
no_error 149
align 16
no_error 150
align 16
no_error 151
align 16
no_error 152
align 16
no_error 153
align 16
no_error 154
align 16
no_error 155
align 16
no_error 156
align 16
no_error 157
align 16
no_error 158
align 16
no_error 159
align 16
no_error 160
align 16
no_error 161
align 16
no_error 162
align 16
no_error 163
align 16
no_error 164
align 16
no_error 165
align 16
no_error 166
align 16
no_error 167
align 16
no_error 168
align 16
no_error 169
align 16
no_error 170
align 16
no_error 171
align 16
no_error 172
align 16
no_error 173
align 16
no_error 174
align 16
no_error 175
align 16
no_error 176
align 16
no_error 177
align 16
no_error 178
align 16
no_error 179
align 16
no_error 180
align 16
no_error 181
align 16
no_error 182
align 16
no_error 183
align 16
no_error 184
align 16
no_error 185
align 16
no_error 186
align 16
no_error 187
align 16
no_error 188
align 16
no_error 189
align 16
no_error 190
align 16
no_error 191
align 16
no_error 192
align 16
no_error 193
align 16
no_error 194
align 16
no_error 195
align 16
no_error 196
align 16
no_error 197
align 16
no_error 198
align 16
no_error 199
align 16
no_error 200
align 16
no_error 201
align 16
no_error 202
align 16
no_error 203
align 16
no_error 204
align 16
no_error 205
align 16
no_error 206
align 16
no_error 207
align 16
no_error 208
align 16
no_error 209
align 16
no_error 210
align 16
no_error 211
align 16
no_error 212
align 16
no_error 213
align 16
no_error 214
align 16
no_error 215
align 16
no_error 216
align 16
no_error 217
align 16
no_error 218
align 16
no_error 219
align 16
no_error 220
align 16
no_error 221
align 16
no_error 222
align 16
no_error 223
align 16
no_error 224
align 16
no_error 225
align 16
no_error 226
align 16
no_error 227
align 16
no_error 228
align 16
no_error 229
align 16
no_error 230
align 16
no_error 231
align 16
no_error 232
align 16
no_error 233
align 16
no_error 234
align 16
no_error 235
align 16
no_error 236
align 16
no_error 237
align 16
no_error 238
align 16
no_error 239
align 16
no_error 240
align 16
no_error 241
align 16
no_error 242
align 16
no_error 243
align 16
no_error 244
align 16
no_error 245
align 16
no_error 246
align 16
no_error 247
align 16
no_error 248
align 16
no_error 249
align 16
no_error 250
align 16
no_error 251
align 16
no_error 252
align 16
no_error 253
align 16
no_error 254
align 16
no_error 255
align 16