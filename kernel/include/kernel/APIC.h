//
// Created by igel on 05.09.24.
//

#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>

uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg);

void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value);

void IOAPIC_setup(void* address,uint8_t APIC_ID);

void write_to_LAPIC(uint64_t offset,uint64_t value);
uint64_t read_from_LAPIC(uint64_t offset);

void end_of_interrupt();

#endif //IOAPIC_H
