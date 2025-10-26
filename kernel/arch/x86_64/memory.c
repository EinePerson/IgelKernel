#include <kernel.h>
#include <stdint.h>
#include <kernel/memory.h>
#include <memory.h>
#include <stddef.h>
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

void* get_virtual_address(struct Memory_Virtual_Address addr) {
    uint64_t ptr = 0;
    ptr |= addr.pt << 12;
    ptr |= addr.pd << 21;
    ptr |= ((uint64_t) addr.pdpt) << 30;
    ptr |= ((uint64_t) addr.pml4) << 39;
    ptr |= ((uint64_t) addr.pml4 >> 8) * 0xFFFF000000000000;
    return (void*)ptr;
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
    bool pdptEnd = (((uint64_t*)cr3)[pml4_index] & 0x40) != 0;
    uint64_t physical_address = (uint64_t) pdpt_base;
    if (!pdptEnd) {
        uint64_t pd_base = pdpt_table[pdpt_index] & 0xFFFFFFFFFFFFF000;
        uint64_t* pd_table = (uint64_t*)(pd_base + PAGE_VIRT_OFFSET);
        bool pdEnd = (pdpt_table[pdpt_index] & 0x40) != 0;
        physical_address = (uint64_t) pd_base;
        if (!pdEnd) {
            uint64_t pt_base = pd_table[pd_index] & 0xFFFFFFFFFFFFF000;
            uint64_t* pt_table = (uint64_t*)(pt_base + PAGE_VIRT_OFFSET);
            physical_address = pt_table[pt_index];
        }
    }

    //TODO remove the removing of last bits if address is bigger
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

/***
 * finds a memory region with the specifications
 * @size is the amount of pages(4096 bytes)
 * @depth is the paging structure depth to search through(currently only 4 is supported)
 */
struct Memory_Virtual_Address find_next_free_page_area(uint8_t depth,uint64_t min_size,uint16_t pml4_start,uint16_t pdpt_start,uint16_t pd_start,uint16_t pt_start) {
    switch (depth) {
        case 4: {
            void* cr3 = get_page_pointer();
            cr3 += PAGE_VIRT_OFFSET;
            uint64_t current_size;
            for (uint16_t i = pml4_start;i < 512;i++) {
                uint64_t pdpt_base = ((uint64_t*)cr3)[i] & 0xFFFFFFFFFFFFF000;
                uint64_t* pdpt_table = (uint64_t*)(pdpt_base + PAGE_VIRT_OFFSET);
                if ((((uint64_t*)cr3)[i] & 0x1) == 0) {
                    current_size += 512 * 512 * 512;
                    if (current_size >= min_size) {
                        struct Memory_Virtual_Address mem = {i,0,0,0};
                        return mem;
                    }
                }else {
                    current_size = 0;
                }
                for (uint16_t j = pdpt_start;j < 512;j++) {
                    if ((pdpt_table[j] & 0x80) != 0)continue;
                    uint64_t pd_base = pdpt_table[j] & 0xFFFFFFFFFFFFF000;
                    uint64_t* pd_table = (uint64_t*)(pd_base + PAGE_VIRT_OFFSET);
                    if ((pdpt_table[j] & 0x1) == 0) {
                        current_size += 512 * 512;
                        if (current_size >= min_size) {
                            struct Memory_Virtual_Address mem = {i,j,0,0};
                            return mem;
                        }
                    }else {
                        current_size = 0;
                    }
                    for (uint16_t k = pd_start;k < 512;k++) {
                        if ((pd_table[k] & 0x80) != 0)continue;
                        uint64_t pt_base = pd_table[k] & 0xFFFFFFFFFFFFF000;
                        uint64_t* pt_table = (uint64_t*)(pt_base + PAGE_VIRT_OFFSET);
                        if ((pd_table[k] & 0x1) == 0) {
                            current_size += 512;
                            if (current_size >= min_size) {
                                struct Memory_Virtual_Address mem = {i,j,k,0};
                                return mem;
                            }
                        }else {
                            current_size = 0;
                        }
                        for (uint16_t l = pt_start;l < 512;l++) {
                            if ((pt_table[l] & 0x1) == 0) {
                                current_size += 1;
                                if (current_size >= min_size) {
                                    struct Memory_Virtual_Address mem = {i,j,k,l};
                                    return mem;
                                }
                            }else {
                                current_size = 0;
                            }
                        }
                    }
                }
            }
            struct Memory_Virtual_Address mem = {-1,-1,-1,-1};
            return mem;
        }
        case 1: {
            void* cr3 = get_page_pointer();
            cr3 += PAGE_VIRT_OFFSET;
            uint64_t current_size;
            for (uint16_t i = pml4_start;i < 512;i++) {
                uint64_t pdpt_base = ((uint64_t*)cr3)[i] & 0xFFFFFFFFFFFFF000;
                uint64_t* pdpt_table = (uint64_t*)(pdpt_base + PAGE_VIRT_OFFSET);
                if ((((uint64_t*)cr3)[i] & 0x1) == 0) {
                    current_size += 512 * 512 * 512;
                    if (current_size >= min_size) {
                        struct Memory_Virtual_Address mem = {i,0,0,0};
                        return mem;
                    }
                }else {
                    current_size = 0;
                }
            }
        }
        default:
            printf("Only page finding level 4 currently supported\n");
            abort();
    }

}

struct Memory_Virtual_Address convertToAddr(uint64_t int_addr){
    return (struct Memory_Virtual_Address){(int_addr >> 39) & 0x1FF,(int_addr >> 30) & 0x1FF,(int_addr >> 21) & 0x1FF,(int_addr >> 12) & 0x1FF};
}