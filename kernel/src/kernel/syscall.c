//
// Created by igel on 10.03.25.
//

#include <stdio.h>
#include <kernel/interrupts.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>

//__attribute__ ((no_caller_saved_registers))
void* syscall_handler(struct interrupt_frame* int_frame,void* rsp){
    printf("Syscall\n");
    struct thread* thread = pop_next_and_readd_running(rsp);
    return thread->rsp;
}
