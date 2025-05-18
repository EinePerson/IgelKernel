//
// Created by igel on 16.05.25.
//

#include <stdint.h>
#include <stdbool.h>
#include <limine.h>
#include <kernel/memory.h>

uint64_t translateAddrToInt(struct Memory_Virtual_Address* addrStr){
    uint64_t addr = addrStr->pml4 | ((addrStr->pml4 & 0x100) * 0x1FFFE);
    addr <<= 9;
    addr |= addrStr->pdpt;
    addr <<= 9;
    addr |= addrStr->pd;
    addr <<= 9;
    addr |= addrStr->pt;
    addr <<= 12;
    return addr;
}