global save_cpu_state
global asm_syscall

global asm_debug
global asm_divide_error
global asm_NMII
global asm_breakP
global asm_overflow
global asm_boundRange
global asm_inv_op
global asm_device_np
global asm_double_fault
global asm_co_seg_over
global asm_inv_tss
global asm_seg_abs
global asm_ss_fault
global asm_prot
global asm_page_fault
global asm_fpu_err
global asm_all_check
global asm_mach_check
global asm_simd
global asn_virt_exc
global asm_con_p_excetpion
global asm_pic_remap
global asm_keyboard_handler
global asm_cascade
global asm_com2
global asm_com1
global asm_ltp2
global asm_floppy_disk
global asm_spurious
global asm_cmos
global asm_periph
global asm_mouse
global asm_copu
global asm_pATA
global asm_sATA
global asm_virt_exc
global asm_APIC_timer_handler
global asm_breakpoint

extern syscall_handler
extern divide_error
extern debug
extern NMII
extern breakP
extern overflow
extern boundRange
extern inv_op
extern device_np
extern double_fault
extern co_seg_over
extern inv_tss
extern seg_abs
extern ss_fault
extern prot
extern page_fault
extern fpu_err
extern all_check
extern mach_check
extern simd
extern virt_exc
extern con_p_excetpion

extern keyboard_handler
extern cascade
extern com2
extern com1
extern ltp2
extern floppy_disk
extern spurious
extern cmos
extern periph
extern mouse
extern copu
extern pATA
extern sATA
extern pic_remap

extern APIC_timer_handler

extern dbg_saveregs
extern dbg_main
extern dbg_loadregs

section .text

%macro begin_interrupt 0
    ;save current state
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdi
    push rdx
    push rcx
    push rbx
    push rax

    ;prepare parameters for function
    lea rdi,[rsp+0x78]
    mov rsi,rsp
%endmacro

%macro end_interrupt 0

    ;set new stack pointer
    mov rsp,rax

    ;restore state
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    iretq
%endmacro

asm_syscall:
    begin_interrupt
    call syscall_handler
    end_interrupt
    NOP

asm_divide_error:
    begin_interrupt
    call divide_error
    end_interrupt
    NOP

asm_debug:
    begin_interrupt
    call debug
    end_interrupt
    NOP

asm_NMII:
    begin_interrupt
    call NMII
    end_interrupt
    NOP

asm_breakP:
    call dbg_saveregs
    call breakP
    mov rdi,3
    call dbg_main
    call dbg_loadregs
    IRETQ

asm_overflow:
    begin_interrupt
    call overflow
    end_interrupt
    NOP

asm_boundRange:
    begin_interrupt
    call boundRange
    end_interrupt
    NOP

asm_inv_op:
    begin_interrupt
    call inv_op
    end_interrupt
    NOP

asm_device_np:
    begin_interrupt
    call device_np
    end_interrupt
    NOP

asm_double_fault:
    begin_interrupt
    call double_fault
    end_interrupt
    NOP

asm_co_seg_over:
    begin_interrupt
    call co_seg_over
    end_interrupt
    NOP

asm_inv_tss:
    begin_interrupt
    call inv_tss
    end_interrupt
    NOP

asm_seg_abs:
    begin_interrupt
    call seg_abs
    end_interrupt
    NOP

asm_ss_fault:
    begin_interrupt
    call ss_fault
    end_interrupt
    NOP

asm_prot:
    begin_interrupt
    call prot
    end_interrupt
    NOP

asm_page_fault:
    begin_interrupt
    call page_fault
    end_interrupt
    NOP

asm_fpu_err:
    begin_interrupt
    call fpu_err
    end_interrupt
    NOP

asm_all_check:
    begin_interrupt
    call all_check
    end_interrupt
    NOP

asm_mach_check:
    begin_interrupt
    call mach_check
    end_interrupt
    NOP

asm_simd:
    begin_interrupt
    call simd
    end_interrupt
    NOP

asm_virt_exc:
    begin_interrupt
    call virt_exc
    end_interrupt
    NOP

asm_con_p_excetpion:
    begin_interrupt
    call con_p_excetpion
    end_interrupt
    NOP

asm_cascade:
    begin_interrupt
    call cascade
    end_interrupt
    NOP

asm_com2:
    begin_interrupt
    call com2
    end_interrupt
    NOP

asm_com1:
    begin_interrupt
    call com1
    end_interrupt
    NOP

asm_ltp2:
    begin_interrupt
    call ltp2
    end_interrupt
    NOP

asm_floppy_disk:
    begin_interrupt
    call floppy_disk
    end_interrupt
    NOP

asm_spurious:
    begin_interrupt
    call spurious
    end_interrupt
    NOP

asm_cmos:
    begin_interrupt
    call cmos
    end_interrupt
    NOP

asm_periph:
    begin_interrupt
    call periph
    end_interrupt
    NOP

asm_mouse:
    begin_interrupt
    call mouse
    end_interrupt
    NOP

asm_copu:
    begin_interrupt
    call copu
    end_interrupt
    NOP

asm_pATA:
    begin_interrupt
    call pATA
    end_interrupt
    NOP

asm_sATA:
    begin_interrupt
    call sATA
    end_interrupt
    NOP

asm_pic_remap:
    begin_interrupt
    call pic_remap
    end_interrupt
    NOP

asm_keyboard_handler:
    begin_interrupt
    call keyboard_handler
    end_interrupt
    NOP

asm_APIC_timer_handler:
    begin_interrupt
    call APIC_timer_handler
    end_interrupt
    NOP