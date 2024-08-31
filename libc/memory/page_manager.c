/*
This is the implementation of the page allocation manager
*/

#include <limine.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <kernel/process.h>

#include "memory.h"
#include <stdio.h>
#include <kernel/memory.h>
#include <kernel.h>

static struct memory* mem;
static uint64_t mem_lenght;
static uint64_t first_free_mem;
static uint64_t first_free_page;//this is index in the first_free_mem memory section
struct process *current_process;
static int64_t offset;

static char same_heap[4096 * 32] __attribute__((aligned(4096)));

//WARNING this does only use memory which is not used otherwise including reclaimable memory(not used)
void init_page(struct limine_memmap_response* memmap){
    init_same(same_heap,4096 * 32,4096);
    offset = get_addresses()->virtual_base - get_addresses()->physical_base;
    mem_lenght = 0;
    first_free_mem = 0;
    first_free_page = 0;
    for(uint64_t i = 0;i < memmap->entry_count;i++){
        mem_lenght += memmap->entries[i]->type == LIMINE_MEMMAP_USABLE;
    }

    mem = malloc(sizeof(struct memory) * mem_lenght);

    uint64_t use_index = 0;
    for(uint64_t i = 0;i < memmap->entry_count;i++){
        if(memmap->entries[i]->type == LIMINE_MEMMAP_USABLE){
            mem[use_index].ptr = (void*) memmap->entries[i]->base;
            mem[use_index].size = memmap->entries[i]->length;

            mem[use_index].used_bits_size = mem[use_index].size / (4096 * 8);
            mem[use_index].used_bits = malloc(mem[use_index].used_bits_size);

            use_index++;
        }
    }

    volatile int j = 0;
}

/*void** alloc_page(uint64_t size) {
    for(uint64_t i = 0;i < size;i++) {

    }
}*/

void* alloc_next_page(uint8_t flags) {
    struct memory* using = mem + first_free_mem;
    using->used_bits[first_free_page / 8] |= (1 << (first_free_page % 8));
    void* ret = using->ptr + 4096 * first_free_page;
    map_next_to(ret,flags);
    find_next_free();
    return ret;
}

void* map_next_to(void *mem,uint8_t flags) {
    if(!check_free_and_alloc()) {
        find_free_virt();
        check_free_and_alloc();
    }
    current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->
        tables[current_process->pd_entry_free]->page[current_process->pt_entry_free] = mem;
    uint64_t virt = current_process->pml4_entry_free & 0x1FF;
    virt = (virt << 9) + (current_process->pdpt_entry_free & 0x1FF);
    virt = (virt << 9) + (current_process->pd_entry_free & 0x1FF);
    virt = (virt << 9) + (current_process->pt_entry_free & 0x1FF);
    virt <<= 12;
    virt |= flags & 0x1F;
    virt |= (flags << 56) & 0x8;

    find_free_virt();

    return (void*) virt;
}

void find_next_free() {
    uint64_t first_free_index = first_free_mem;
    uint64_t arr_index = first_free_page / 8;
    uint8_t offset = first_free_page % 8;
    while (first_free_index < mem_lenght) {
        struct memory* free = mem + first_free_index;
        while(arr_index < free->used_bits_size) {
            bool isFree = free->used_bits[arr_index] >> offset;
            if(isFree) {
                first_free_page = arr_index * 8 + offset;
                first_free_mem = first_free_index;
                return;
            }
            offset++;
            offset = offset % 8;
            arr_index += offset == 0;
        }
        first_free_index++;
        arr_index = 0;
        offset = 0;
    }
    printf("Out of page memory");
    abort();
}

void find_free_virt() {
    current_process->pt_entry_free++;
    current_process->pd_entry_free += current_process->pt_entry_free == 512;
    current_process->pt_entry_free %= 512;
    current_process->pdpt_entry_free += current_process->pd_entry_free == 512;
    current_process->pd_entry_free %= 512;
    current_process->pml4_entry_free +=  current_process->pdpt_entry_free == 512;
     current_process->pdpt_entry_free %= 512;
    if(current_process->pml4_entry_free == 512) {
        printf("Out of virtual memory space");
        abort();
    }

    while (current_process->pml4_entry_free < 512) {
        while ( current_process->pdpt_entry_free < 512) {
            while (current_process->pd_entry_free < 512) {
                while (current_process->pt_entry_free < 512) {
                    if(check_free_and_alloc()) {
                        return;
                    }
                    current_process->pt_entry_free++;
                }
                current_process->pd_entry_free++;
            }
             current_process->pdpt_entry_free++;
        }
        current_process->pml4_entry_free++;
    }

    printf("Out of virtual memory space");
    abort();
}

bool check_free_and_alloc() {
    if((uint64_t) get_pml4() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free] = malloc_same() - offset;
    }
    if((uint64_t) get_pdpt() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free] = malloc_same();
    }
    if((uint64_t) get_pd() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->
        tables[current_process->pd_entry_free] = malloc_same() -offset;
    }

    if((uint64_t) get_pt() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->
        tables[current_process->pd_entry_free]->page[current_process->pt_entry_free] = malloc_same() - offset;
    }

    return current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->
        tables[current_process->pd_entry_free]->page[current_process->pt_entry_free] == 0;
}

void set_current_process(struct process* process) {
    current_process = process;
}

void * get_pt() {
    return get_pd()->page[current_process->pd_entry_free] + offset;
}

struct page_table * get_pd() {
    return  get_pdpt()->tables[current_process->pd_entry_free] + offset;
}

struct page_dir_pointer* get_pml4() {
    return current_process->pml4->dirs[current_process->pml4_entry_free] + offset;
}

struct page_directory* get_pdpt() {
    return get_pml4()->dirs[current_process->pdpt_entry_free] + offset;
}




