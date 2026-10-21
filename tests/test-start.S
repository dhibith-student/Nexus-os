.section .text
.global _start
.type _start, @function
_start:
    call main
    movl %eax, %edi
    movl $60, %eax
    syscall
    hlt
