//
// Created by igel on 31.08.24.
//

#include <stdint.h>

#include <stdio.h>

/*struct interrupt_frame
{
    uint64_t ip;
    uint64_t cs;
    uint64_t flags;
    uint64_t sp;
    uint64_t ss;
};*/

static void hcf(void) {
    asm("cli");
    for (;;) {
        asm ("hlt");
    }
}

void reserved(struct interrupt_frame *frame) {
    hcf();
}

__attribute__ ((interrupt))
void divide_error(struct interrupt_frame *frame){
    printf("Divide by 0 exception");
    hcf();
}

__attribute__ ((interrupt))
void debug(struct interrupt_frame *frame){
    printf("Debug");
}

__attribute__ ((interrupt))
void NMII(struct interrupt_frame *frame){
    printf("Non maskable external interrupt");
    hcf();
}

__attribute__ ((interrupt))
void breakP(struct interrupt_frame *frame){
    printf("breakP");
}

__attribute__ ((interrupt))
void overflow(struct interrupt_frame *frame){
    printf("Overflow");
    hcf();
}

__attribute__ ((interrupt))
void bound(struct interrupt_frame *frame){
    printf("Bound range exceded");
    hcf();
}

__attribute__ ((interrupt))
void inv_op(struct interrupt_frame *frame){
    printf("Invalid Opcode");
    hcf();
}

__attribute__ ((interrupt))
void device_np(struct interrupt_frame *frame){
    printf("Device not Available(FPU)");
    hcf();
}

__attribute__ ((interrupt))
void double_fault(struct interrupt_frame *frame){
    printf("Double fault");
    hcf();
}

__attribute__ ((interrupt))
void co_seg_over(struct interrupt_frame *frame){
    printf("Coprocessor overrun");
    hcf();
}

__attribute__ ((interrupt))
void inv_tss(struct interrupt_frame *frame){
    printf("Invalid TSS");
    hcf();
}

__attribute__ ((interrupt))
void seg_abs(struct interrupt_frame *frame){
    printf("Segment not present");
    hcf();
}

__attribute__ ((interrupt))
void ss_fault(struct interrupt_frame *frame){
    printf("Stack Segment Fault");
    hcf();
}

__attribute__ ((interrupt))
void prot(struct interrupt_frame *frame){
    printf("General Protection");
    hcf();
}

__attribute__ ((interrupt))
void page_fault(struct interrupt_frame *frame){
    printf("Page Fault");
    hcf();
}

__attribute__ ((interrupt))
void fpu_err(struct interrupt_frame *frame){
    printf("Floating-point error");
    hcf();
}

__attribute__ ((interrupt))
void all_check(struct interrupt_frame *frame){
    printf("Alignment Check");
    hcf();
}

__attribute__ ((interrupt))
void mach_check(struct interrupt_frame *frame){
    printf("Machine CHeck");
    hcf();
}

__attribute__ ((interrupt))
void simd(struct interrupt_frame *frame){
    printf("SIMD FP Exception");
    hcf();
}

__attribute__ ((interrupt))
void virt_exc(struct interrupt_frame *frame){
    printf("Virtualisation exception");
    hcf();
}

__attribute__ ((interrupt))
void con_p_excetpion(struct interrupt_frame *frame){
    printf("Control Protection Exception");
    hcf();
}