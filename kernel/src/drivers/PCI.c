//
// Created by igel on 07.09.24.
//

#include <kernel.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <drivers/PCI.h>
#include <kernel/memory.h>
#include <kernel/RSDP.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

static uint32_t entrie_count;
static uint16_t pml4_index;

static struct PCIe_device* devices;
static uint32_t device_count;

void * get_PCIe_address(struct PCIe_device *device,uint8_t function) {
    uint64_t addr = 0xFFFF;
    addr <<= 9;
    addr |= pml4_index;
    addr <<= 9;
    addr |= device->group & 0x1FF;
    addr <<= 9;
    addr |= device->bus & 0x1FF;
    addr <<= 9;
    addr |= ((device->device & 0x1F) << 3) | (function & 0x7);
    addr <<= 12;

    return (void*)addr;
}

void PCIe_init() {
    struct PCIe* pcie = getPCIe();
    if(pcie == 0) {
        printf("No PCIe");
        abort();
    }


    entrie_count = (pcie->header.Length - sizeof(struct PCIe)) / sizeof(struct PCIe_entry);
    struct PCIe_entry* entries = pcie->entries;
    pml4_index = next_free_pml4();
    device_count = 0;
    for(uint32_t i = 0;i < entrie_count;i++) {
        device_count += get_device_count(&entries[i]);
    }

    devices = malloc(device_count * sizeof(struct PCIe_device));
    uint64_t device_id = -1;
    for(uint32_t i = 0;i < entrie_count;i++) {
        init_devices(&device_id,&i,&entries[i]);
    }

    map_devices(entries);
}

uint32_t get_device_count(struct PCIe_entry* entry) {
    void* cr3 = get_page_pointer();
    volatile void* pdptr = malloc_same();
    cr3 += PAGE_VIRT_OFFSET;
    ((uint64_t**)cr3)[pml4_index] = (uint64_t*) ((uint64_t) ((uint8_t*) pdptr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    volatile void* pd = malloc_same();
    ((uint64_t**)pdptr)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pd - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    volatile void* pt = malloc_same();
    ((uint64_t**)pd)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pt - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    ((uint64_t**)pt)[511] = (uint64_t*) (entry->base_addr | MEMORY_MAPPED_IO_FLAGS);

    uint64_t virt = 0xFFFF;
    virt <<= 9;
    virt += pml4_index;
    virt <<= 9;
    virt += 511;
    virt <<= 9;
    virt += 511;
    virt <<= 9;
    virt += 511;
    virt <<= 12;
    void* addr = (void*)virt;

    uint32_t count = 0;
    uint64_t addr_add = 0;
    for(uint32_t i = entry->start_pci;i < entry->end_pci;i++) {
        for(uint8_t j = 0;j < 32;j++) {
            for(uint8_t k = 0;k < 8;k++) {
                invalidate_addr(addr);
                ((uint64_t**)pt)[511] = (uint64_t*) ((entry->base_addr + addr_add) | MEMORY_MAPPED_IO_FLAGS);
                if(((uint32_t*)addr)[0] != 0xFFFFFFFF/*Invalid vendor ID*/) {
                    count++;
                    addr_add += (8 - i) * 4096;
                    break;
                }
                addr_add += 4096;//=0x1000 offset of one part
            }
        }
    }

    free_same(pt);
    free_same(pd);
    free_same(pdptr);
    ((uint64_t**)cr3)[pml4_index] = (uint64_t*)0;

    return count;
}

void init_devices(uint64_t *id_ptr,uint32_t* group_id, struct PCIe_entry *entry) {
    void* cr3 = get_page_pointer();
    volatile void* pdptr = malloc_same();
    cr3 += PAGE_VIRT_OFFSET;
    ((uint64_t**)cr3)[pml4_index] = (uint64_t*) ((uint64_t) ((uint8_t*) pdptr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    volatile void* pd = malloc_same();
    ((uint64_t**)pdptr)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pd - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    volatile void* pt = malloc_same();
    ((uint64_t**)pd)[511] = (uint64_t*) ((uint64_t) ((uint8_t*) pt - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    ((uint64_t**)pt)[511] = (uint64_t*) (entry->base_addr | MEMORY_MAPPED_IO_FLAGS);

    uint64_t virt = 0xFFFF;
    virt <<= 9;
    virt += pml4_index;
    virt <<= 9;
    virt += 511;
    virt <<= 9;
    virt += 511;
    virt <<= 9;
    virt += 511;
    virt <<= 12;
    void* addr = (void*)virt;

    uint64_t addr_add = 0;
    for(uint32_t i = entry->start_pci;i < entry->end_pci;i++) {
        for(uint8_t j = 0;j < 32;j++) {
            for(uint8_t k = 0;k < 8;k++) {
                invalidate_addr(addr);
                ((uint64_t**)pt)[511] = (uint64_t*) ((entry->base_addr + addr_add) | MEMORY_MAPPED_IO_FLAGS);
                if(((uint32_t*)addr)[0] != 0xFFFFFFFF/*Invalid vendor ID*/) {
                    if(devices[id_ptr[0]].supported_functions != 0) {
                        id_ptr[0]++;
                        devices[id_ptr[0]].bus = i;
                        devices[id_ptr[0]].device = j;
                        devices[id_ptr[0]].supported_functions |= 1 << k;
                        devices->group = entry->PCI_segment_group;
                    }else {
                        devices[id_ptr[0]].supported_functions |= 1 << k;
                    }
                }
                addr_add += 4096;//=0x1000 offset of one part
            }
        }
    }

    free_same(pt);
    free_same(pd);
    free_same(pdptr);
    ((uint64_t**)cr3)[pml4_index] = (uint64_t*)0;
}

void map_devices(struct PCIe_entry* entry) {
    void* cr3 = get_page_pointer();
    cr3 += PAGE_VIRT_OFFSET;
    volatile void* pdptr;
    if(((uint64_t*)cr3)[pml4_index] == 0) {
        pdptr = malloc_same();
        ((uint64_t**)cr3)[pml4_index] = (uint64_t*) ((uint64_t) ((uint8_t*) pdptr - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
    }else {
        pdptr = (void*)((uint64_t) (((char*) ((uint64_t**)cr3)[pml4_index]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
    }

    for(uint32_t i = 0;i < device_count;i++) {
        uint16_t group = devices[i].group;
        volatile void* pd;
        if(((uint64_t*) pdptr)[group] == 0) {
            pd = malloc_same();
            ((uint64_t**)pdptr)[group] = (uint64_t*) ((uint64_t) ((uint8_t*) pd - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
        }else {
            pd = (void*)((uint64_t) (((char*) ((uint64_t**)pdptr)[group]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
        }

        uint16_t bus = devices[i].bus;
        volatile void* pt;
        if(((uint64_t*)pd)[bus] == 0) {
            pt = malloc_same();
            ((uint64_t**)pd)[bus] = (uint64_t*) ((uint64_t) ((uint8_t*) pt - get_kernel_virt_offset()) | MEMORY_MAPPED_IO_FLAGS);
        }else {
            pt = (void*)((uint64_t) (((char*) ((uint64_t**)pd)[bus]) + get_kernel_virt_offset()) & 0xFFFFFFFFFFFFF000);
        }

        uint16_t device = ((devices[i].device & 0x1F) << 3);
        for(uint8_t j = 0;j < 8;j++) {
            if((devices[i].supported_functions >> j) & 1) {
                uint16_t id = device + j;
                ((uint64_t**)pt)[id] = (uint64_t*) (entry[group].base_addr + id * 4096 + 0x100000 * bus | MEMORY_MAPPED_IO_FLAGS);
            }
        }
    }
}

uint32_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    // Write out the address
    outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = inl(0xCFC);//(uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint32_t get_PCIe_device_count() {
    return device_count;
}

struct PCIe_device * get_PCIe_device_list() {
    return devices;
}
