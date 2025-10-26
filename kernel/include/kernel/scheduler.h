//
// Created by igel on 10.06.25.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

/*
 *Because the threads in the scheduler change and move often all are saved in a linked list
 */

//this is the structure that saves the data relevant for scheduling
struct list_data{
    struct thread *thread;
    uint64_t score;//this is the data which is used to save various data determined by the list and function you are in including the time until the thread is to be awoken and the priority in scheduling
    struct list_data *next;
};

///this retrieves the next thread to be scheduled and reinserts the currently running thread into the queue
struct thread* pop_next_and_readd_running(void* rsp);

///this retrieves the next thread to schedule and removes it from the scheduler
struct thread* pop_next_thread();

void add_thread_to_schedule(struct thread* thread);

void awake_thread(uint64_t tID);

void this_wait();

void this_sleep(uint64_t delta_time);

void this_kill();

struct thread* spawn_child_thread(void* start_instruction_address);

void scheduler_init();

void set_scheduler_function(void (*scheduler_function_new)(struct list_data*, struct list_data*));

void init_kernel_thread();

#endif //SCHEDULER_H