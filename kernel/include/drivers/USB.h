//
// Created by igel on 07.09.24.
//

#ifndef USB_H
#define USB_H

struct xHCI_capabilities {
    uint8_t length;
    uint8_t reserved;
    uint16_t version;
    uint32_t s_param_1;
    uint32_t s_param_2;
    uint32_t s_param_3;
    uint32_t c_param_1;
    uint32_t doorbell_offset;
    uint32_t rrso;
    uint32_t c_param_2;
}__attribute__((packed));

void usb_init();

#endif //USB_H
