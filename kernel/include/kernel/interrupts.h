#pragma once

struct interrupt_frame
{
    uint16_t ip;
    uint16_t cs;
    uint16_t flags;
    uint16_t sp;
    uint16_t ss;
};

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

void PITI(struct interrupt_frame *frame);
//void keyboard(struct interrupt_frame *frame);
void cascade(struct interrupt_frame *frame);
void com2(struct interrupt_frame *frame);
void com1(struct interrupt_frame *frame);
void ltp2(struct interrupt_frame *frame);
void floppy_disk(struct interrupt_frame *frame);
void spurious(struct interrupt_frame *frame);
void cmos(struct interrupt_frame *frame);
void periph(struct interrupt_frame *frame);
void mouse(struct interrupt_frame *frame);
void copu(struct interrupt_frame *frame);
void pATA(struct interrupt_frame *frame);
void sATA(struct interrupt_frame *frame);

void pic_remap(struct interrupt_frame *frame);