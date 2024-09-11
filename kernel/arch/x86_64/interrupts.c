#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <drivers/keyboard.h>
#include <kernel/APIC.h>

#include <kernel/interrupts.h>

#define IDT_MAX_DESCRIPTORS 64

__attribute__ ((interrupt))
void PITI(struct interrupt_frame *frame) {
    printf("PITI");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void cascade(struct interrupt_frame *frame) {
    printf("Cascade");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void com2(struct interrupt_frame *frame) {
    printf("COM2");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void com1(struct interrupt_frame *frame) {
    printf("COM1");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void ltp2(struct interrupt_frame *frame) {
    printf("LPT2");
    end_of_interrupt();
}

void floppy_disk(struct interrupt_frame *frame) {
    printf("FLOPPY");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void spurious(struct interrupt_frame *frame) {
    printf("supurios");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void cmos(struct interrupt_frame *frame) {
    printf("CMOS");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void periph(struct interrupt_frame *frame) {
    printf("Peripheral");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void mouse(struct interrupt_frame *frame) {
    printf("mouse");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void copu(struct interrupt_frame *frame) {
    printf("Coprocessor");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void pATA(struct interrupt_frame *frame) {
    printf("Primary ATA");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void sATA(struct interrupt_frame *frame) {
    printf("Secondary ATA");
    end_of_interrupt();
}

__attribute__ ((interrupt))
void pic_remap(struct interrupt_frame *frame) {
    printf("PIC");
    end_of_interrupt();
}

void* interrupt_pointers[IDT_MAX_DESCRIPTORS] = {divide_error,debug,NMII,breakP,overflow,bound,inv_op,device_np,double_fault,co_seg_over,inv_tss,seg_abs,ss_fault,prot,page_fault,
    reserved,fpu_err,all_check,mach_check,simd,virt_exc,con_p_excetpion,
    reserved,reserved,reserved,reserved,reserved,reserved,reserved,reserved,reserved,reserved,
    //PIC remaped
    pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,pic_remap,
    //APIC
    PITI,keyboard_handler,cascade,com2,com1,ltp2,floppy_disk,spurious,cmos,periph,periph,periph,mouse,copu,pATA,sATA};

typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isr_high;     // The higher 32 bits of the ISR's address
    uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry;

typedef struct {
    uint16_t	limit;
    uint64_t	base;
} __attribute__((packed)) idtr_t;

__attribute__((aligned(0x10)))
static idt_entry idt[256];

static idtr_t idtr;

__attribute__((noreturn))
void exception_handler() {
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    //TODO replace this with dynamic offset
    descriptor->kernel_cs      = 0x0008;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < IDT_MAX_DESCRIPTORS; vector++) {
        idt_set_descriptor(vector, interrupt_pointers[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}