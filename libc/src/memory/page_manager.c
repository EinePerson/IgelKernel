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
static uint64_t offset;

//static char same_heap[4096 * 32] __attribute__((aligned(4096)));

/**
 * This function finds an area within the usable memory and assigns it to the virtual address space of the kernel
 */
void* find_page_area() {
    struct Memory_Virtual_Address addr = find_next_free_page_area(4,32,256,0,0,0);
    /*bool physFound = false;
    uint16_t page_free_start = -1;
    struct page_level_4* pl4 = (struct page_level_4*) ((((uint64_t) get_page_pointer()) & 0x0007FFFFFFFFF000) + PAGE_VIRT_OFFSET);
    struct page_dir_pointer* pdpt = (struct page_dir_pointer*) ((((uint64_t) pl4->dirs[256]) & 0x0007FFFFFFFFF000) + PAGE_VIRT_OFFSET);
    struct page_directory* pd = (struct page_directory*) ((((uint64_t) pdpt->dirs[0]) & 0x0007FFFFFFFFF000) + PAGE_VIRT_OFFSET);
    //struct page_table* pt = (struct page_table*) ((((uint64_t) pd->tables[0]) & 0x0007FFFFFFFFF000) + PAGE_VIRT_OFFSET);
    struct page_table* pt = (struct page_table*) pd;
    for (uint16_t i = 0;i < 480;i++) {
        bool empty = true;
        for (uint8_t j = 0;j < 32;j++) {
            if (pt->page[i + j] != 0) {
                empty = false;
                break;
            }
        }
        if (empty) {
            physFound = true;
            page_free_start = i;
            break;
        }
    }

    if (!physFound) {
        printf("Unable to locate correct page\n");
        abort();
    }

    bool found = false;
    uint64_t mem_area_id = -1;
    uint64_t mem_area_offset = -1;
    for (uint64_t i = 0; i < mem_lenght; i++) {
        if ((uint64_t) mem[i].ptr <= page_free_start * 4096 && ((uint64_t) mem[i].ptr) <= page_free_start * 4096 + 4096 * 32 ) {
        //if (mem[i].size >= 4096 * 32) {
            found = true;
            mem_area_id = i;
            mem_area_offset = (uint64_t) mem[i].ptr / 4096;
            break;
        }
    }

    if (!found) {
        printf("No memory region is big enough");
        hcf();
    }*/

    /*if ((uint64_t) mem[mem_area_id].ptr & ~0xFFF != 0) {
        //TODO align to page boundaries
        printf("Unaligned memory region\n");
        hcf();
    }

    if (mem[mem_area_id].used_bits[0] + mem[mem_area_id].used_bits[1] + mem[mem_area_id].used_bits[2] + mem[mem_area_id].used_bits[3] != 0) {
        //TODO move memory region(this should all be free)
        printf("Memory already used\n");
        hcf();
    }*/

    /*for (uint8_t i = 0;i < 32;i++) {
        mem[mem_area_id].used_bits[(mem_area_offset + i) / 8] |= (1 << (7 - (mem_area_offset + i) % 8));
    }

    for (uint8_t i = 0;i < 32;i++) {
        pt->page[page_free_start + i] = (void*) ((i + mem_area_offset) * 4096 + ((uint64_t) mem[mem_area_id].ptr));
    }

    return (void*) (PAGE_VIRT_OFFSET + (page_free_start << 12));*/

    /*uint64_t virt_address = PAGE_VIRT_OFFSET + (uint64_t) mem[mem_area_id].ptr;
    uint16_t page = (virt_address >> 12) & 0x1FF;
    uint16_t page_table = (virt_address >> 21) & 0x1FF;
    uint16_t page_directory = (virt_address >> 30) & 0x1FF;
    uint16_t page_directory_table = (virt_address >> 39) & 0x1FF;*/
    volatile void* virt_addr = get_virtual_address(addr);
    volatile int j = 0;
}

//WARNING this does only use memory which is not used otherwise including reclaimable memory(not used)
void init_page(struct limine_memmap_response* memmap){

    //TODO for kernel the offset needs to be PAGE_VIRT_OFFSET,if a new block(user thread etc)is started it needs to be this
    offset = get_addresses()->virtual_base - get_addresses()->physical_base;
    //offset = PAGE_VIRT_OFFSET;
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

    void* heap_ptr = find_page_area();

    init_same(heap_ptr,4096 * 32,4096);
}



/*void** alloc_page(uint64_t size) {
    for(uint64_t i = 0;i < size;i++) {

    }
}*/

/**
 * allocates a new page from physical memory
 * @return the physical address of the address space
 */
void* alloc_physical_page() {
    struct memory* using = mem + first_free_mem;
    using->used_bits[first_free_page / 8] |= (1 << (first_free_page % 8));
    void* ret = using->ptr + 4096 * first_free_page;
    find_next_free();
    return ret;
}

/**
 * Allocates a new page from physical memory and maps it to the next free virtual address range
 * @param flags the flags of the virtual memory shall hold
 * @return the virtual address where the allocated area start
 */
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

/**
 * This function finds the next free virtual address space of the current process
 */
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

/**
 * This function checks weather the values given in the process where free virtual address space is located are true and allocates paging structures when needed
 * @return weather the value is free
 */
bool check_free_and_alloc() {
    if((uint64_t) get_pml4() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free] = malloc_same() - offset;
    }
    if((uint64_t) get_pdpt() == offset) {
        current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free] = malloc_same() - offset;
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
    uint64_t pd = (uint64_t) get_pd();
    if (pd == offset)return (void*) offset;
    return (void*) ((uint64_t) get_pd()->page[current_process->pd_entry_free] + offset);
}

struct page_table* get_pd() {
    uint64_t pdpt = (uint64_t) get_pdpt();
    if (pdpt == offset)return (struct page_table*) offset;
    return (struct page_table*) ((uint64_t)get_pdpt()->tables[current_process->pd_entry_free] + offset);
}

struct page_dir_pointer* get_pml4() {
    if (!(current_process && current_process->pml4)) return (struct page_dir_pointer*) offset;
    return (struct page_dir_pointer*) ((uint64_t) current_process->pml4->dirs[current_process->pml4_entry_free] + offset);
}

struct page_directory* get_pdpt() {
    struct page_dir_pointer* pdb = get_pml4();
    if ((uint64_t) pdb == offset)return (struct page_directory*) offset;
    return (struct page_directory*) ((uint64_t) get_pml4()->dirs[current_process->pdpt_entry_free] + offset);
}




