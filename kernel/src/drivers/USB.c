//
// Created by igel on 07.09.24.
//

#include <stdio.h>
#include <stdlib.h>
#include <drivers/PCI.h>
#include <drivers/USB.h>
#include <kernel/memory.h>

static struct PCIe_device* usb_device;
static struct PCIe_Header* usb_header;

void usb_init() {
    struct PCIe_device* devices = get_PCIe_device_list();
    uint32_t device_count = get_PCIe_device_count();
    for(uint32_t i = 0;i < device_count;i++) {
        void* address = get_PCIe_address(&devices[i],0);
        struct PCIe_Header* header = (struct PCIe_Header*)address;
        if(header->class_code == 0x0C && header->subClass == 0x03 && header->prog_IF == 0x30) {
            usb_device = devices + i;
            usb_header = header;
            break;
        }
    }

    if(usb_device == 0) {
        printf("No USB device present");
        abort();
    }

    struct xHCI_capabilities* capabilities = map_virtual_address(usb_header->BAR0);
    uint64_t xECP = capabilities->c_param_1 >> 16;
}
