#include <kernel.h>
#include <stdint.h>
#include <kernel/memory.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct Memory_Virtual_Address disassemble_virtual_address(void* addr) {
    uint64_t virutal_address = (uint64_t)addr;
    uint64_t pml4_index = (virutal_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virutal_address >> 30) & 0x1FF;
    uint64_t pd_index   = (virutal_address >> 21) & 0x1FF;
    uint64_t pt_index   = (virutal_address >> 12) & 0x1FF;
    struct Memory_Virtual_Address mem = {pml4_index,pdpt_index,pd_index,pt_index};
    return mem;
}

void * map_virtual_address(void *physical_address) {
    /*if(((uint64_t) physical_address & 0xFFF) != 0) {
        printf("Address not page-aligned");
        abort();
    }*/
    static int16_t pml4 = -1;
    static int16_t pdpt = 511;
    static int16_t pd = 511;
    static int16_t pt = 511;
    if(pml4 == (int16_t) -1)pml4 = next_free_pml4();
    if(pt < 0) {
        pt = 511;
        pd--;
    }
    if(pd < 0) {
        pd = 511;
        pdpt--;
    }
    if(pdpt < 0) {
        pml4 = next_free_pml4();
        pdpt = 511;
    }

    volatile void* cr3 = get_page_pointer();
    cr3 += PAGE_VIRT_OFFSET;

    volatile void* pdptr;
    if(((uint64_t*)cr3)[pml4] == 0) {
        pdptr = malloc_same();
        ((uint64_t**)cr3)[pml4] = (uint64_t*) ((uint64_t) ((uint8_t*) pdptr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    }else {
        pdptr = (void*)((uint64_t) (((char*) ((uint64_t**)cr3)[pml4]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
    }

    volatile void* pdPtr;
    if(((uint64_t*) pdptr)[pdpt] == 0) {
        pdPtr = malloc_same();
        ((uint64_t**)pdptr)[pdpt] = (uint64_t*) ((uint64_t) ((uint8_t*) pdPtr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    }else {
        pdPtr = (void*)((uint64_t) (((char*) ((uint64_t**)pdptr)[pd]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
    }

    volatile void* ptPtr;
    if(((uint64_t*)pdPtr)[pd] == 0) {
        ptPtr = malloc_same();
        ((uint64_t**)pdPtr)[pd] = (uint64_t*) ((uint64_t) ((uint8_t*) ptPtr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    }else {
        ptPtr = (void*)((uint64_t) (((char*) ((uint64_t**)pdPtr)[pd]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
    }
    ((uint64_t**)ptPtr)[pt] = (uint64_t*) ((uint64_t) physical_address & ~0xFFF | MEMORY_MAPPED_IO_FLAGS);

    uint64_t virt = 0xFFFF;
    virt <<= 9;
    virt += pml4;
    virt <<= 9;
    virt += pdpt;
    virt <<= 9;
    virt += pd;
    virt <<= 9;
    virt += pt;
    virt <<= 12;
    virt += (uint64_t) physical_address & 0xFFF;

    pt--;

    return (void*) virt;
}

void* get_phys_addr(void* virt_addr){
    void* cr3 = get_page_pointer();
    cr3 += PAGE_VIRT_OFFSET;

    uint64_t virutal_address = (uint64_t)virt_addr;
    uint64_t pml4_index = (virutal_address >> 39) & 0x1FF;
    uint64_t pdpt_index = (virutal_address >> 30) & 0x1FF;
    uint64_t pd_index   = (virutal_address >> 21) & 0x1FF;
    uint64_t pt_index   = (virutal_address >> 12) & 0x1FF;

    uint64_t pdpt_base = ((uint64_t*)cr3)[pml4_index] & 0xFFFFFFFFFFFFF000;
    uint64_t* pdpt_table = (uint64_t*)(pdpt_base + PAGE_VIRT_OFFSET);
    uint64_t pd_base = pdpt_table[pdpt_index] & 0xFFFFFFFFFFFFF000;
    uint64_t* pd_table = (uint64_t*)(pd_base + PAGE_VIRT_OFFSET);
    uint64_t pt_base = pd_table[pd_index] & 0xFFFFFFFFFFFFF000;
    uint64_t* pt_table = (uint64_t*)(pt_base + PAGE_VIRT_OFFSET);
    uint64_t physical_address = pt_table[pt_index];

    if(physical_address >> 49 & 0x0001) {
        physical_address |= 0xFFFF000000000000;
        physical_address &= 0xFFFFFFFFFFFFF000;
    }else {
        physical_address &= 0x0000FFFFFFFFF000;
    }

    physical_address += (virutal_address & 0xFFF);//Offset
    return (void*) physical_address;
}

uint16_t next_free_pml4() {
    struct page_existing* pep = get_existens_map(get_page_pointer() + PAGE_VIRT_OFFSET,1);
    for(uint16_t i = 511;i > 0;i--) {
        if(pep->exists_ptr[i] == 0) {
            free(pep);
            return i;
        }
    }
    free(pep);
    printf("Out of pml4 slots for process");
    abort();
}

void invalidate_addr(void *virt_addr) {
    asm volatile("invlpg (%0)" ::"r" (virt_addr) : "memory");
}

struct page_existing * get_existens_map_default() {
    void* cr3 = get_page_pointer();
    cr3 += PAGE_VIRT_OFFSET;
    return get_existens_map(cr3,4);
}

struct page_existing* get_existens_map(void* cr3,int8_t depth) {
    if(depth <= 0)
        return (struct page_existing*) 1;
    depth--;
    struct page_existing* exs = malloc(sizeof(struct page_existing));


    for(uint16_t i = 0;i < 512;i++) {
        if(i == 511 && depth == 3) {
            printf("Test");
        }
        if((((uint64_t*) cr3)[i] & 0xFFFFFFFFFFFFF000) != 0) {
            void* ptr = (void*) (((uint64_t*) cr3)[i] & 0xFFFFFFFFFFFFF000);
            ptr += PAGE_VIRT_OFFSET;
            exs->exists_ptr[i] = get_existens_map(ptr,depth);
        }else exs->exists_ptr[i] = 0;
    }

    return exs;
}
