#pragma once

void idt_init();

void reserved(struct interrupt_frame *frame);

void divide_error(struct interrupt_frame *frame);
void debug(struct interrupt_frame *frame);
void NMII(struct interrupt_frame *frame);
void breakP(struct interrupt_frame *frame);
void overflow(struct interrupt_frame *frame);
void bound(struct interrupt_frame *frame);
void inv_op(struct interrupt_frame *frame);
void device_np(struct interrupt_frame *frame);
void double_fault(struct interrupt_frame *frame);
void co_seg_over(struct interrupt_frame *frame);
void inv_tss(struct interrupt_frame *frame);
void seg_abs(struct interrupt_frame *frame);
void ss_fault(struct interrupt_frame *frame);
void prot(struct interrupt_frame *frame);
void page_fault(struct interrupt_frame *frame);
void fpu_err(struct interrupt_frame *frame);
void all_check(struct interrupt_frame *frame);
void mach_check(struct interrupt_frame *frame);
void simd(struct interrupt_frame *frame);
void virt_exc(struct interrupt_frame *frame);
void con_p_excetpion(struct interrupt_frame *frame);
