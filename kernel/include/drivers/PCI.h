//
// Created by igel on 07.09.24.
//

#ifndef PCI_H
#define PCI_H

#include <kernel/RSDP.h>

//memory layout for memory mapped PCIe devices (the rest is unsupported)

/*
 * 12 least significant(registers offset) is the page(as they are the same size)
 * 9 (page table index) device and function
 * 9 (page directory) bus number
 * 9 (page directory pointer) group
 */
struct PCIe_device {
    uint16_t group;
    uint8_t bus;
    uint8_t device;
    uint16_t supported_functions;
};

struct PCIe_Header {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision;
    uint8_t prog_IF;
    uint8_t subClass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency;
    uint8_t header;
    uint8_t BIST;
    void* BAR0;
    void* BAR2;
    void* BAR4;
} __attribute__((packed));

void* get_PCIe_address(struct PCIe_device* device,uint8_t function);

void PCIe_init();

uint32_t get_device_count(struct PCIe_entry* entry);

void init_devices(uint64_t* id_ptr,uint32_t* group_id,struct PCIe_entry* entry);

void map_devices(struct PCIe_entry* entry);

uint32_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

struct PCIe_device* get_PCIe_device_list();
uint32_t get_PCIe_device_count();

#endif //PCI_H
