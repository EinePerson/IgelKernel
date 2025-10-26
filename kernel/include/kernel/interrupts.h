#pragma once

#include <stdint.h>

struct interrupt_frame{
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    void* sp;
    uint64_t ss;
};

void idt_init();

void reserved(struct interrupt_frame *frame);

void divide_error(struct interrupt_frame *frame);
extern void asm_divide_error();
void debug(struct interrupt_frame *frame);
extern void asm_debug();
void NMII(struct interrupt_frame *frame);
extern void asm_NMII();
void breakP(struct interrupt_frame *frame);
extern void asm_breakP();
void overflow(struct interrupt_frame *frame);
extern void asm_overflow();
void boundRange(struct interrupt_frame *frame);
extern void asm_boundRange();
void inv_op(struct interrupt_frame *frame);
extern void asm_inv_op();
void device_np(struct interrupt_frame *frame);
extern void asm_device_np();
void double_fault(struct interrupt_frame *frame);
extern void asm_double_fault();
void co_seg_over(struct interrupt_frame *frame);
extern void asm_co_seg_over();
void inv_tss(struct interrupt_frame *frame);
extern void asm_inv_tss();
void seg_abs(struct interrupt_frame *frame);
extern void asm_seg_abs();
void ss_fault(struct interrupt_frame *frame);
extern void asm_ss_fault();
void prot(struct interrupt_frame *frame);
extern void asm_prot();
void page_fault(struct interrupt_frame *frame);
extern void asm_page_fault();
void fpu_err(struct interrupt_frame *frame);
extern void asm_fpu_err();
void all_check(struct interrupt_frame *frame);
extern void asm_all_check();
void mach_check(struct interrupt_frame *frame);
extern void asm_mach_check();
void simd(struct interrupt_frame *frame);
extern void asm_simd();
void virt_exc(struct interrupt_frame *frame);
extern void asm_virt_exc();
void con_p_excetpion(struct interrupt_frame *frame);
extern void asm_con_p_excetpion();

void PITI(struct interrupt_frame *frame);
extern void asm_PITI();
//void keyboard(struct interrupt_frame *frame);
extern void asm_keyboard_handler();
void cascade(struct interrupt_frame *frame);
extern void asm_cascade();
void com2(struct interrupt_frame *frame);
extern void asm_com2();
void com1(struct interrupt_frame *frame);
extern void asm_com1();
void ltp2(struct interrupt_frame *frame);
extern void asm_ltp2();
void floppy_disk(struct interrupt_frame *frame);
extern void asm_floppy_disk();
void spurious(struct interrupt_frame *frame);
extern void asm_spurious();
void cmos(struct interrupt_frame *frame);
extern void asm_cmos();
void periph(struct interrupt_frame *frame);
extern void asm_periph();
void mouse(struct interrupt_frame *frame);
extern void asm_mouse();
void copu(struct interrupt_frame *frame);
extern void asm_copu();
void pATA(struct interrupt_frame *frame);
extern void asm_pATA();
void sATA(struct interrupt_frame *frame);
extern void asm_sATA();
void pic_remap(struct interrupt_frame *frame);
extern void asm_pic_remap();
extern void asm_syscall(struct interrupt_frame *frame);

void* syscall_handler(struct interrupt_frame* int_frame,void* rsp);
extern void asm_APIC_timer_handler();
void* APIC_timer_handler(struct interrupt_frame* int_frame,void* rsp);