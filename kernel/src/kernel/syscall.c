//
// Created by igel on 10.03.25.
//

#include <stdio.h>
#include <kernel/interrupts.h>
#include <kernel/process.h>

__attribute__ ((interrupt))
void syscall_handler(struct interrupt_frame *frame){
    //printf("Test\n");
    //volatile int i = 0;
    save_cpu_state();
    printf("Syscall\n");
}
