//
// Created by igel on 28.08.24.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_VIRT_OFFSET 0xffff800000000000//This is the offset of the physical address to the virtual one in bootloader reclaimable memory
#define MEMORY_MAPPED_IO_FLAGS 0b11011//these are the page flags for memory mapped IO devices

struct Memory_Virtual_Address {
    uint16_t pml4;
    uint16_t pdpt;
    uint16_t pd;
    uint16_t pt;
};

struct Memory_Virtual_Address disassemble_virtual_address(void* addr);

///Maps the respective physical address to a virtuall address
void* map_virtual_address(void* physical_address);

void* get_page_pointer();

void* get_phys_addr(void* virt_addr);

struct page_existing* get_existens_map_default();
struct page_existing* get_existens_map(void* cr3,int8_t depth);

uint16_t next_free_pml4();

void invalidate_addr(void* virt_addr);

#endif //MEMORY_H
