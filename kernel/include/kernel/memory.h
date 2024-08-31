//
// Created by igel on 28.08.24.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_VIRT_OFFSET 0xffff800000000000//This is the offset of the physical address to the virtual one in bootloader reclaimable memory

void* get_page_pointer();

void* get_phys_addr(void* virt_addr);

struct page_existing* get_existens_map_default();
struct page_existing* get_existens_map(void* cr3,int8_t depth);

#endif //MEMORY_H
