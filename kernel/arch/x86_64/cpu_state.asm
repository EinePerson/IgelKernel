global save_cpu_state
global asm_syscall

extern syscall_handler

section .text

asm_syscall:
    call save_cpu_state
    call syscall_handler
    RET

save_cpu_state:
    ;push rax
    ;push rbx
    ;push rcx
    ;push rdx
    ;push rsi
    ;push r8
    ;push r9
    ;push r10
    ;push r11
    ;push r12
    ;push r13
    ;push r14
    ;push r15
    ;pushfq
    ;MOV RAX,CS
    ;PUSH RAX
    ;MOV RAX,DS
    ;PUSH RAX
    ;MOV RAX,ES
    ;PUSH RAX
    ;MOV RAX,FS
    ;PUSH RAX
    ;MOV RAX,GS
    ;PUSH RAX
    RET