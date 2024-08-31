#include <stdint.h>
#include <kernel/memory.h>
#include <memory.h>
#include <stdio.h>

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
