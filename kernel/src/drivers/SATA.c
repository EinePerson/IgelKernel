//
// Created by igel on 12.09.24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drivers/PCI.h>
#include <drivers/SATA.h>
#include <kernel/memory.h>

static uint32_t used_ports = 0;
static uint32_t present_ports = 0;

void SATA_init() {
    struct PCIe_device* sata = find_PCIe_device(0x01,0x06);
    if(!sata) {
        printf("No sATA");
        abort();
    }
    struct PCIe_Header* header = get_PCIe_address(sata,0);
    struct SATA_HBA_MEM* mem = map_virtual_address((void*) (((uint32_t*)header)[0x24 / 4]));

    present_ports = mem->pi;
    for(uint8_t i = 0;i < 32;i++) {
        if(!(present_ports >> i & 0x01))continue;
        used_ports |= ((mem->ports[i].ssts & 0x3) != 0) << i;
    }


}
