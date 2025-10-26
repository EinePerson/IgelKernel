//
// Created by igel on 30.08.25.
//

#include <stddef.h>

#include "kernel/scheduler.h"
#include <stdint.h>

void standart_FCFS_scheduler(struct list_data* list,struct list_data* to_insert){
    struct list_data* first = list;
    while (first->next)first = first->next;
    first->next = to_insert;
}

void scheduler_init(){
    set_scheduler_function(&standart_FCFS_scheduler);
    init_kernel_thread();
}
