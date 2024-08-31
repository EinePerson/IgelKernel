#include "memory.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

static struct free_memory free_mem;

void init_heap(void* start,uint64_t size){
    free_mem.size = 0;
    free_mem.next_block = start;

    ((struct free_memory*) start)->next_block = 0;
    ((struct free_memory*) start)->size = size;
}

void* malloc(uint64_t size){
    struct free_memory* previous = 0;
    struct free_memory* next_free = free_mem.next_block;
    while (next_free != 0){
        if(next_free->size >= size + sizeof(struct used_metadata)){
            break;
        }
        previous = next_free;
        next_free = next_free->next_block;
    }

    if(next_free == 0) {
        printf("Heap out of space");
        abort();
    }

    if(next_free == size + sizeof(struct used_metadata)){
        struct used_metadata* meta = (struct used_metadata*)next_free;
        void* mem = (void*)next_free + sizeof(struct used_metadata);
        memset(mem,0,size);

        //TODO insert error for previous or next_free null
        previous->next_block = next_free->next_block;

        meta->size = size + sizeof(struct used_metadata);
        return mem;
    }

    struct used_metadata* meta = (struct used_metadata*)(((void*)next_free) + next_free->size - size - sizeof(struct used_metadata));
    void* mem = (void*)meta + sizeof(struct used_metadata);
    memset(mem,0,size);
    next_free->size -= size + sizeof(struct used_metadata);

    meta->size = size + sizeof(struct used_metadata);
    return mem;
}

void free(void* ptr){
    struct used_metadata* meta = (struct used_metadata*) (ptr - sizeof(struct used_metadata));
    struct free_memory* prev = 0;
    struct free_memory* next = free_mem.next_block;
    while (next < ptr && next != 0){
        prev = next;
        next = next->next_block;
    }

    memset(ptr,0,meta->size - sizeof(struct used_metadata));
    if(prev == 0)prev = free_mem.next_block;

    struct free_memory* assigned;

    if(((void*)prev) + prev->size == meta){
        //add this to the previous free block
        prev->size += meta->size;
        assigned = prev;
    }else{
        uint64_t size = meta->size;
        //if this cannot be added to the preceding region
        struct free_memory* freed = (struct free_memory*)meta;
        freed->next_block = prev->next_block;
        freed->size = size;
        prev->next_block = freed;
        assigned = freed;
    }

    if(((void*)assigned) + assigned->size == assigned->next_block){
		//add the proceding block to this
		struct free_memory* proc = assigned->next_block;
        assigned->size += assigned->next_block->size;
        assigned->next_block = assigned->next_block->next_block;
        proc->next_block = 0;
        proc->size = 0;
    }
}