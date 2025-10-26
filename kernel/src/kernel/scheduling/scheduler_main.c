//
// Created by igel on 25.05.25.
//

#include <stdbool.h>
#include <stdint.h>
#include <memory.h>
#include <stddef.h>

#include "kernel/APIC.h"
#include "kernel/process.h"
#include "kernel/scheduler.h"
#include "dbg.h"
#include "string.h"

//if this is set there is currently nothing to schedule
static bool empty_scheduling;
static struct list_data* next_running_thread;
uint64_t next_count;

//these threads are waiting upon other events
static struct list_data* next_waiting_thread;
uint64_t waiting_count;

static struct list_data* next_sleeping_thread;
uint64_t sleeping_count;


//this is storage to prevent hig amounts of memory (de)allocation
static struct list_data* empty_thread_data;
uint64_t empty_count;

//this is not saved in a list format but the list structure is kept because it is probably going to be inserted into another list after it is finished
static struct list_data* current_running_thread;
//static uint64_t start_time;

//this is the thread id the next process will get
//it is never decremented meaning that during runtime there can not exist more than 2^64 threads(which I do not think is reached that easily)
static uint64_t thread_id_inc = 0;

//this is the scheduler function which can be dynamically set,the first parameter is the list where each scheduled thread is contained within,the second one is the one to be inserted
static void (*scheduler_function)(struct list_data*,struct list_data*);

///this is sets the scheduler function which can be dynamically set,the first parameter is the list where each scheduled thread is contained within,the second one is the one to be inserted
void set_scheduler_function(void (*scheduler_function_new)(struct list_data*,struct list_data*)){
    scheduler_function = scheduler_function_new;
}

struct thread* pop_next_and_readd_running(void* rsp){
    current_running_thread->thread->rsp = rsp;
    if (empty_scheduling)return current_running_thread->thread;
    current_running_thread->thread->state = ready;
    scheduler_function(next_running_thread,current_running_thread);
    next_count++;
    return pop_next_thread();
}

struct thread* pop_next_thread(){
    struct list_data *element = next_running_thread;
    struct thread *thread = element->thread;
    next_running_thread = element->next;
    next_count--;
    empty_scheduling = next_count == 0;

    element->next = 0;

    current_running_thread = element;

    thread->state = running;
    return thread;
}

void malloc_empty_bucket(){
    struct list_data* bucket = malloc(sizeof(struct list_data) * 10);
    for (char i = 0; i < 9; i++){
        bucket[i].next = &bucket[i + 1];
    }
    //TODO check commented code
    //if (empty_count == 0)
        bucket[9].next = empty_thread_data;
    empty_thread_data = bucket;
    empty_count += 10;
}

void add_thread_to_schedule(struct thread* thread){
    thread->state = ready;

    if(empty_count == 0)malloc_empty_bucket();
    struct list_data* to_use = empty_thread_data;
    to_use->thread = thread;
    empty_thread_data = to_use->next;
    empty_count--;
    to_use->next = NULL;

    if (next_count == 0)next_running_thread = to_use;
    else scheduler_function(next_running_thread,to_use);
    next_count++;

    empty_scheduling = false;
}

void awake_thread(uint64_t tID){
    if (next_sleeping_thread->thread->tid == tID){
        struct list_data* current = next_sleeping_thread;
        current->thread->state = ready;
        scheduler_function(next_running_thread,current);
        next_count++;
        return;
    }

    struct list_data* current = next_sleeping_thread;
    while (current != NULL && current->thread->tid != tID){
        if (current->thread->tid == tID)break;
        current = current->next;
    }

    if (current == NULL)return;

    current->thread->state = ready;
    scheduler_function(next_running_thread,current);
    next_count++;
}

void this_wait(){
    current_running_thread->thread->state = waiting;
    current_running_thread->next = next_waiting_thread;
    next_waiting_thread = current_running_thread;
    waiting_count++;
}

void this_sleep(uint64_t delta){
    current_running_thread->thread->state = sleeping;
    current_running_thread->next = next_sleeping_thread;
    current_running_thread->score = delta;
    next_sleeping_thread = current_running_thread;
    sleeping_count++;

    add_to_sleep(delta,current_running_thread->thread->tid);
}

void this_kill(){
    current_running_thread->thread->state = dead;
    current_running_thread->thread = 0;
    current_running_thread->score = 0;

    current_running_thread->next = empty_thread_data;
    empty_thread_data = current_running_thread;
    empty_count++;
}

struct thread* spawn_child_thread(void* start_instruction_address){
    struct thread* thread = malloc(sizeof(struct thread));
    thread->state = un_started;
    thread->parent_pid = current_running_thread->thread->parent_pid;
    thread->tid = thread_id_inc++;
    thread->flags = 0;
    void** start_stack = alloc_next_free_page_directory();
    uint64_t* stack = (uint64_t*) ((uint8_t*) start_stack + 0x1000 - 0xA0);
    memset(stack,0,20 * 8);
    stack[15] = (uint64_t) start_instruction_address;
    stack[16] = 0x8;
    stack[17] = 0x102;
    stack[18] = (uint64_t) stack + 0xA0;
    stack[19] = 0x10;
    
    //stack += 15/*General Purpose registers*/ + 5/*ISR stack pushed objects*/;
    thread->rsp = stack;

    return thread;
}

void init_kernel_thread(){
    malloc_empty_bucket();
    current_running_thread = empty_thread_data;
    empty_thread_data = empty_thread_data->next;
    empty_count--;

    current_running_thread->next = 0;
    current_running_thread->thread = malloc(sizeof(struct thread));
    current_running_thread->thread->state = running;
    current_running_thread->thread->tid = 0;
    current_running_thread->thread->flags = 0;
    current_running_thread->thread->parent_pid = 0;
}