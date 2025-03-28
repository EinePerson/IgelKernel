//
// Created by igel on 29.08.24.
//

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

extern void save_cpu_state();

struct register_data {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

struct thread_data {
    struct register_data regs;
    void* cr3;
    struct thread_data* next;
};

struct process{
    uint64_t pid;
    struct page_level_4* pml4;
    uint32_t processor_id;//this value should always be set once started to act cache friendly and not change processor
    uint16_t pml4_entry_free;//this are the indexes to where the next free page region of each level starts
    uint16_t pdpt_entry_free;
    uint16_t pd_entry_free;
    uint16_t pt_entry_free;

    uint64_t flags;
    bool superviser;//if this is ring-0(1) or ring-3(0)
};

#endif //PROCESS_H
