.section .text
.extern exception_dispatch

.macro ISR_NO_ERROR name, vector
.global \name
.type \name, @function
\name:
    pushq $0
    pushq $\vector
    jmp exception_common
.endm

.macro ISR_ERROR name, vector
.global \name
.type \name, @function
\name:
    pushq $\vector
    jmp exception_common
.endm

ISR_NO_ERROR exception_divide_error, 0
ISR_NO_ERROR exception_invalid_opcode, 6
ISR_ERROR exception_double_fault, 8
ISR_ERROR exception_general_protection, 13
ISR_ERROR exception_page_fault, 14

exception_common:
    cli
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, %rdi
    call exception_dispatch
.Lexception_halt:
    hlt
    jmp .Lexception_halt
