//
// Created by igel on 05.09.24.
//

#include <kernel/APIC.h>
#include <kernel/MSR.h>

uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg){
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xff);
    return ioapic[4];
}

void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value){
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xff);
    ioapic[4] = value;
}

void IOAPIC_setup(void *address,uint8_t APIC_ID) {
    for(uint8_t i = 0 ;i < 16;i++) {
        uint32_t val = (0x30 + i) & 0x00FF;
        val |= (0x1 << 16) * (i == 2);//for some reason the cascade is triggered and is thus masked here
        cpuWriteIoApic(address,0x10 + 2 * i,val);
        cpuWriteIoApic(address,0x14 + 2 * i,APIC_ID << 24);
    }
}

void write_to_LAPIC(uint64_t offset, uint64_t value) {
    write_msr(0x800 + offset,value);
}

uint64_t read_from_LAPIC(uint64_t offset) {
    return read_msr(0x800 + offset);
}

void end_of_interrupt() {
    write_to_LAPIC(0xb,0);
}
