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
#include <kernel.h>
#include <kernel/memory.h>

static struct memory* mem;
static uint64_t mem_lenght;
static uint64_t first_free_mem;
static uint64_t first_free_page;//this is index in the first_free_mem memory section
struct process *current_process;
//static uint64_t offset;

static struct address_space_translator* translator;

static uint64_t INTERNAL_MEMORY_OFFSET = 0;
//this is for the page structure memory area
static struct page_level_4* paging_pml4;
static struct page_dir_pointer paging_pdpt;
static struct page_directory paging_pd;
static struct page_table paging_pt;

//static char same_heap[4096 * 32] __attribute__((aligned(4096)));

void map_address_space(uint16_t pml4_id,uint16_t pdpt_id,uint16_t pd_id,uint16_t pt_id,void* physical_address,uint64_t length,uint16_t flags) {
    uint64_t current_length = 0;
    struct page_dir_pointer* pml4 = get_pml4(pml4_id);
    struct page_directory* pdpt = get_pdpt(pml4_id,pdpt_id);
    struct page_table* pd = get_pd(pml4_id,pdpt_id,pd_id);
    uint64_t* pt = get_pt(pml4_id,pdpt_id,pt_id,pt_id);
    while(current_length < length) {
        if (pt_id >= 512) {
            pt_id = 0;
            pd_id++;
            pd = get_pd(pml4_id,pdpt_id,pd_id);
            pt = get_pt(pml4_id,pdpt_id,pt_id,pt_id);
        }
        if (pd_id >= 512) {
            pd_id = 0;
            pdpt_id++;
            pdpt = get_pdpt(pml4_id,pdpt_id);
            pd = get_pd(pml4_id,pdpt_id,pd_id);
            uint64_t* page = ((uint64_t*) pd->page);
            page[pt_id] &= (uint64_t) (~0xFFF);
            page[pt_id] |= (uint64_t) (flags & 0xFFF);
            pt = get_pt(pml4_id,pdpt_id,pt_id,pt_id);
        }

        if (pdpt_id >= 512) {
            pdpt_id = 0;
            pml4_id++;
            pml4 = get_pml4(pml4_id);
            pdpt = get_pdpt(pml4_id,pdpt_id);
            pd = get_pd(pml4_id,pdpt_id,pd_id);
            pt = get_pt(pml4_id,pdpt_id,pt_id,pt_id);
        }

        pd->page[pt_id] = (void*) (((uint64_t) physical_address + 4096 * current_length) & ~0xFFF | (flags & 0xFFF));

        current_length++;
        pt_id++;
    }
}

/**
 * This function finds an area within the usable memory and assigns it to the virtual address space of the kernel
 */
void* find_page_area() {
    struct Memory_Virtual_Address addr = find_next_free_page_area(1,32,256,0,0,0);

    void* virt_addr = get_virtual_address(addr);
    void* phys_addr = alloc_physical_page_range(32);
    paging_pml4 = get_page_pointer() + PAGE_VIRT_OFFSET;
    paging_pml4->dirs[addr.pml4] = (void*) (((uint64_t) &paging_pdpt) - INTERNAL_MEMORY_OFFSET | DEFAULT_KERNEL_PAGE_FLAGS);
    paging_pdpt.dirs[addr.pdpt] = (void*) (((uint64_t) &paging_pd) - INTERNAL_MEMORY_OFFSET | DEFAULT_KERNEL_PAGE_FLAGS);
    paging_pd.tables[addr.pd] = (void*) (((uint64_t) &paging_pt) - INTERNAL_MEMORY_OFFSET | DEFAULT_KERNEL_PAGE_FLAGS);
    for (uint16_t i = 0; i < 32; i++) {
        paging_pt.page[i] = (void*) ((((uint64_t) phys_addr) + i * 4096) | DEFAULT_KERNEL_PAGE_FLAGS);
    }

    translator_add_entry(translator,virt_addr,phys_addr,32 * 4096);
    return virt_addr;
}

//WARNING this does only use memory which is not used otherwise including reclaimable memory(not used)
void init_page(struct limine_memmap_response* memmap, const struct limine_kernel_address_response* address_range){
    INTERNAL_MEMORY_OFFSET = address_range->virtual_base - address_range->physical_base;
    translator = create_translator();
    translator_init_kernel_space(translator);
    //TODO for kernel the offset needs to be PAGE_VIRT_OFFSET,if a new block(user thread etc)is started it needs to be this
    //offset = get_addresses()->virtual_base - get_addresses()->physical_base;
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

void markUsedArea(uint64_t memory_index,uint64_t arr_index,uint8_t offset,uint64_t length){
    struct memory* using = mem + memory_index;
    for (uint64_t i = 0; i < length; i++) {
        using->used_bits[arr_index + i / 8] |= (1 << (arr_index + i + offset) % 8);
    }

    if (first_free_mem == memory_index) {
        if (first_free_page == arr_index * 8 + offset)first_free_page += length;
        if ((mem + memory_index)->size <= first_free_page) {
            first_free_page = 0;
            first_free_mem = 0;
        }
    }
}

void* alloc_physical_page_range(uint64_t length) {
    uint64_t current_size = 0;
    uint64_t arr_index = 0;
    uint8_t offset = 0;

    for (uint64_t i = 0;i < mem_lenght;i++) {
        struct memory current_mem = mem[i];
        for (uint64_t j = 0;j < current_mem.used_bits_size;j++) {
            for (uint8_t k = 0;k < 8;k++) {
                if (current_mem.used_bits[j] == 0xFF) {
                    arr_index = j;
                    offset = k;
                    continue;
                }
                if (current_mem.used_bits[j] & (1 << k)) {
                    current_size = 0;
                    arr_index = j;
                    offset = k;
                }else {
                    current_size++;
                    if (current_size >= length) {
                        markUsedArea(i,arr_index,offset,length);
                        return (void*) (((uint64_t) current_mem.ptr) + (((arr_index * 8) + offset) * 4096));
                    }
                }
            }
        }
    }

    return (void*) MEMORY_INVALID_RETURN;
}

/**
 * Allocates a new page from physical memory and maps it to the next free virtual address range
 * @param flags the flags of the virtual memory shall hold
 * @return the virtual address where the allocated area start
 */
void* alloc_next_page(uint8_t flags) {
    struct memory* using = mem + first_free_mem;
    using->used_bits[first_free_page / 8] |= (1 << (first_free_page % 8));
    void* phys = using->ptr + 4096 * first_free_page;
    void* virt = map_next_to(phys,flags);
    find_next_free();
    return virt;
}

void* map_next_to(void *mem,uint64_t flags) {
    flags &= SECURITY_FLAG_MASK;
    struct Memory_Virtual_Address addr = get_current_process_address();
    if(!check_free_and_alloc_structures(&addr,flags)) {
        find_free_virt();
        addr = get_current_process_address();
        check_free_and_alloc_structures(&addr,flags);
    }
    struct page_table* pt = get_pd(current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free);
    pt->page[current_process->pt_entry_free] = (void*) (((uint64_t) mem) | flags);
    //current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->
      //  tables[current_process->pd_entry_free]->page[current_process->pt_entry_free] = mem;
    /*uint64_t virt = current_process->pml4_entry_free & 0x1FF;
    virt = (virt << 9) + (current_process->pdpt_entry_free & 0x1FF);
    virt = (virt << 9) + (current_process->pd_entry_free & 0x1FF);
    virt = (virt << 9) + (current_process->pt_entry_free & 0x1FF);
    virt <<= 12;
    virt |= flags & 0x1F;
    virt |= (flags << 56) & 0x8;*/

    void* virt = get_virtual_address(addr);

    translator_add_entry(translator,virt,mem,0x1000);

    find_free_virt();

    return virt;
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
    uint64_t virt_addr = (uint64_t) get_virtual_address(get_current_process_address());
    if(current_process->pml4_entry_free == 512) {
        printf("Out of virtual memory space");
        abort();
    }

    if(current_process->pml4_entry_free < 256){
        printf("Lower half kernel not supported");
        abort();
    }

    while (virt_addr > 0){
        if(translator_contains_immideate_virtual_addr(translator,virt_addr,1)){
            virt_addr += 0x8000000000;
            virt_addr &= ~0x7FFFFFFFFF;
            continue;
        }
        if(translator_contains_immideate_virtual_addr(translator,virt_addr,2)){
            virt_addr += 0x40000000;
            virt_addr &= ~0x3FFFFFFF;
            continue;
        }
        if(translator_contains_immideate_virtual_addr(translator,virt_addr,3)){
            virt_addr += 0x200000;
            virt_addr &= ~0x100000;
            continue;
        }
        if(translator_contains_immideate_virtual_addr(translator,virt_addr,4)){
            virt_addr += 0x1000;
            continue;
        }
        current_process->pt_entry_free = 0x1FF & (virt_addr >> 12);
        current_process->pd_entry_free = 0x1FF & (virt_addr >> 21);
        current_process->pdpt_entry_free = 0x1FF & (virt_addr >> 30);
        current_process->pml4_entry_free = 0x1FF & (virt_addr >> 39);
        return;
    }
    

    /*while (current_process->pml4_entry_free < 512) {
        uint64_t current_PML4 = (uint64_t) get_pml4(current_process->pml4_entry_free);

        if(current_PML4 == MEMORY_INVALID_RETURN) {
            //struct Memory_Virtual_Address sub = {current_process->pml4_entry_free,0,0,0};
            //check_free_and_alloc(&sub);
            return;
        }
        while (current_process->pdpt_entry_free < 512) {
            uint64_t currentPDPT = (uint64_t) get_pdpt(current_process->pml4_entry_free,current_process->pdpt_entry_free);
            if (currentPDPT == MEMORY_END_OF_TRAVERSAL_RETURN) {
                current_process->pdpt_entry_free++;
                continue;
            }
            if(currentPDPT == MEMORY_INVALID_RETURN) {
                //struct Memory_Virtual_Address sub = {current_process->pml4_entry_free,current_process->pdpt_entry_free,0,0};
                //check_free_and_alloc(&sub);
                return;
            }
            while (current_process->pd_entry_free < 512) {
                uint64_t currentPD = (uint64_t) get_pd(current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free);
                if (currentPD == MEMORY_END_OF_TRAVERSAL_RETURN) {
                    current_process->pd_entry_free++;
                    continue;
                }
                if(currentPD == MEMORY_INVALID_RETURN) {
                    //struct Memory_Virtual_Address sub = {current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free,0};
                    //check_free_and_alloc(&sub);
                    return;
                }
                while (current_process->pt_entry_free < 512) {
                    uint64_t currentPT = (uint64_t) get_pt(current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free,current_process->pt_entry_free);
                    if(currentPT == MEMORY_INVALID_RETURN) {
                        //struct Memory_Virtual_Address sub = {current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free,current_process->pt_entry_free};
                        //check_free_and_alloc(&sub);
                        return;
                    }
                    if (((uint64_t) current_process->pml4->dirs[current_process->pml4_entry_free]->dirs[current_process->pdpt_entry_free]->tables[current_process->pd_entry_free]->
                        page[current_process->pt_entry_free]) & 0x1 == 0)return;
                    current_process->pt_entry_free++;
                }
                current_process->pt_entry_free = 0;
                current_process->pd_entry_free++;
            }
            current_process->pd_entry_free = 0;
             current_process->pdpt_entry_free++;
        }
        current_process->pdpt_entry_free = 0;
        current_process->pml4_entry_free++;
    }*/

    printf("Out of virtual memory space");
    abort();
}

/**
 * This function checks weather the values given in the process where free virtual address space is located are true and allocates paging structures when needed
 * @return weather the value is free
 */
bool check_free_and_alloc(struct Memory_Virtual_Address* addr) {
    uint64_t prev = 0;
    uint64_t current = (uint64_t) get_pml4(addr->pml4);
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        current_process->pml4->dirs[addr->pml4] = translator_translate_virtual(translator,newEntry);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pdpt(addr->pml4,addr->pdpt);
    if (current == MEMORY_END_OF_TRAVERSAL_RETURN)return false;
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
       ((struct page_dir_pointer*) prev)->dirs[addr->pdpt] = translator_translate_virtual(translator,newEntry);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pd(addr->pml4,addr->pdpt,addr->pd);
    if (current == MEMORY_END_OF_TRAVERSAL_RETURN)return false;
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        ((struct page_directory*) prev)->tables[addr->pd] = translator_translate_virtual(translator,newEntry);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pt(addr->pml4,addr->pdpt,addr->pd,addr->pt);
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        ((struct page_table*) prev)->page[addr->pt] = translator_translate_virtual(translator,newEntry);
    }

    return ((uint64_t) ((struct page_table*) prev)->page[addr->pt]) & 0x1 == 0;
}

bool check_free_and_alloc_structures(struct Memory_Virtual_Address* addr,uint16_t flags) {
    if(translator_contains_virtual_addr(translator,addr,4)) return false;

    uint64_t current = (uint64_t) get_pml4(addr->pml4) & ~0xFFF;
    if(!translator_contains_virtual_addr(translator,addr,1)){
        void* newEntry = malloc_same();
        current_process->pml4->dirs[addr->pml4] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        void* prev = newEntry;

        newEntry = malloc_same();
        ((struct page_dir_pointer*) prev)->dirs[addr->pdpt] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        prev = (void*) newEntry;

        newEntry = malloc_same();
        ((struct page_directory*) prev)->tables[addr->pd] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        return true;
    }
    if(!translator_contains_virtual_addr(translator,addr,2)){
        void* prev = (void*) ((uint64_t) get_pdpt(addr->pml4,addr->pdpt) & ~0xFFF);
        void* newEntry = malloc_same();
        ((struct page_dir_pointer*) prev)->dirs[addr->pdpt] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        prev = (void*) newEntry;

        newEntry = malloc_same();
        ((struct page_directory*) prev)->tables[addr->pd] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        return true;
    }

    if(!translator_contains_virtual_addr(translator,addr,3)){
        void* prev = (void*) ((uint64_t) get_pd(addr->pml4,addr->pdpt,addr->pd) & ~0xFFF);
        void* newEntry = malloc_same();
        ((struct page_directory*) prev)->tables[addr->pd] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
    }
    return true;
    /*struct Memory_Virtual_Address tst = {258,0,0,0};
    uint64_t prev = 0;
    uint64_t current = (uint64_t) get_pml4(addr->pml4) & ~0xFFF;
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        current_process->pml4->dirs[addr->pml4] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pdpt(addr->pml4,addr->pdpt) & ~0xFFF;
    if (current == MEMORY_END_OF_TRAVERSAL_RETURN)return false;
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        ((struct page_dir_pointer*) prev)->dirs[addr->pdpt] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pd(addr->pml4,addr->pdpt,addr->pd) & ~0xFFF;
    if (current == MEMORY_END_OF_TRAVERSAL_RETURN)return false;
    if(current == MEMORY_INVALID_RETURN) {
        void* newEntry = malloc_same();
        ((struct page_directory*) prev)->tables[addr->pd] = (void*) ((uint64_t) translator_translate_virtual(translator,newEntry) | flags);
        current = (uint64_t) newEntry;
    }
    prev = current;
    current = (uint64_t) get_pt(addr->pml4,addr->pdpt,addr->pd,addr->pt) & ~0xFFF;
    if(current == MEMORY_INVALID_RETURN) {
        return true;
    }

    return ((uint64_t) ((struct page_table*) prev)->page[addr->pt]) & 0x1 == 0;*/
}

void set_current_process(struct process* process) {
    current_process = process;
}

struct Memory_Virtual_Address get_current_process_address() {
    struct Memory_Virtual_Address address = {current_process->pml4_entry_free,current_process->pdpt_entry_free,current_process->pd_entry_free,current_process->pt_entry_free};
    return address;
}

void * get_pt(uint16_t pml4_id,uint16_t pdpt_id,uint16_t pd_id,uint16_t pt_id) {
    uint64_t pd = (uint64_t) get_pd(pml4_id,pdpt_id,pd_id);
    if (pd == MEMORY_INVALID_RETURN)return (void*) MEMORY_INVALID_RETURN;
    if (pd == MEMORY_END_OF_TRAVERSAL_RETURN)return (void*) MEMORY_INVALID_RETURN;
    //if ((translator_get_flags_virtually(translator,(void*) pd,0) & 0x1) != 0x1)return (void*) MEMORY_ALREADY_PRESENT_RETURN;
    if (translator_contains_physically(translator,get_pd(pml4_id,pdpt_id,pd_id)->page[pt_id]))return (void*) MEMORY_INVALID_RETURN;
    return (void*) ((uint64_t) translator_translate_physical(translator,get_pd(pml4_id,pdpt_id,pd_id)->page[pt_id]) & ~0XFFF);
}

struct page_table* get_pd(uint16_t pml4_id,uint16_t pdpt_id,uint16_t pd_id) {
    uint64_t pdpt = (uint64_t) get_pdpt(pml4_id,pdpt_id);
    if (pdpt == MEMORY_INVALID_RETURN)return (struct page_table*) MEMORY_INVALID_RETURN;
    if (pdpt == MEMORY_END_OF_TRAVERSAL_RETURN)return (struct page_table*) MEMORY_INVALID_RETURN;
    if (!translator_contains_physically(translator,get_pdpt(pml4_id,pdpt_id)->tables[pd_id]))return (struct page_table*) MEMORY_INVALID_RETURN;
    //TODO hope this works with index 1 and not 2
    //if ((translator_get_flags_virtually(translator,(void*) pdpt,1) & 0x1) != 0x1)return (struct page_table*) MEMORY_ALREADY_PRESENT_RETURN;
    if ((translator_get_level_flags(translator,(void*) pdpt,1) & 0x80) == 0x80) return (struct page_table*) MEMORY_END_OF_TRAVERSAL_RETURN;
    return (struct page_table*) ((uint64_t) translator_translate_physical(translator,get_pdpt(pml4_id,pdpt_id)->tables[pd_id]) & ~0XFFF);
}

struct page_dir_pointer* get_pml4(uint16_t pml4_id) {
    if (!(current_process && current_process->pml4)) return (struct page_dir_pointer*) MEMORY_INVALID_RETURN;
    if (!translator_contains_physically(translator,current_process->pml4->dirs[pml4_id])) return (struct page_dir_pointer*) MEMORY_INVALID_RETURN;
    //if ((translator_get_flags_virtually(translator,(void*) current_process->pml4->dirs[pml4_id],1) & 0x1) != 0x1)return (struct page_dir_pointer*) MEMORY_ALREADY_PRESENT_RETURN;
    return (struct page_dir_pointer*) ((uint64_t) translator_translate_physical(translator,current_process->pml4->dirs[pml4_id]) & ~0XFFF);
}

struct page_directory* get_pdpt(uint16_t pml4_id,uint16_t pdpt_id) {
    struct page_dir_pointer* pdb = get_pml4(pml4_id);
    if ((uint64_t) pdb == MEMORY_INVALID_RETURN)
        return (struct page_directory*) MEMORY_INVALID_RETURN;
    if (!translator_contains_physically(translator,get_pml4(pml4_id)->dirs[pdpt_id]))
        return (struct page_directory*) MEMORY_INVALID_RETURN;
    //if ((translator_get_flags_virtually(translator,(void*) pdb,1) & 0x1) != 0x1)return (struct page_directory*) MEMORY_ALREADY_PRESENT_RETURN;
    if ((translator_get_level_flags(translator,(void*) pdb,2) & 0x80) == 0x80)
        return (struct page_directory*) MEMORY_END_OF_TRAVERSAL_RETURN;
    struct page_directory* ptr = (struct page_directory*) ((uint64_t) translator_translate_physical(translator,get_pml4(pml4_id)->dirs[pdpt_id]) & ~0XFFF);
    return ptr;
}
