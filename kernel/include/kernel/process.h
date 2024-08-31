//
// Created by igel on 29.08.24.
//

#ifndef PROCESS_H
#define PROCESS_H

struct process{
    uint64_t pid;
    struct page_level_4* pml4;
    uint32_t processor_id;//this value should always be set once started to act cache frendly and not change processor
    uint16_t pml4_entry_free;//this are the indexes to where the next free page region of each level starts
    uint16_t pdpt_entry_free;
    uint16_t pd_entry_free;
    uint16_t pt_entry_free;

    bool superviser;//if this is ring-0(1) or ring-3(0)
};

#endif //PROCESS_H
