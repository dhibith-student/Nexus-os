.section .text
.global _start
.type _start, @function
_start:
    cld
    call kernel_main
.Lhalt:
    hlt
    jmp .Lhalt
.size _start, . - _start
