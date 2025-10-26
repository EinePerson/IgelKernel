//
// Created by igel on 05.09.24.
//

#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>

enum LAPIC_Registers;

uint32_t cpuReadIoApic(void *ioapicaddr, uint32_t reg);

void cpuWriteIoApic(void *ioapicaddr, uint32_t reg, uint32_t value);

void IOAPIC_setup(void* address,uint8_t APIC_ID);

void write_to_LAPIC(enum LAPIC_Registers offset,uint64_t value);
uint64_t read_from_LAPIC(enum LAPIC_Registers offset);

void end_of_interrupt();

uint64_t messurre_LAPIC_timer();

void add_to_sleep(uint64_t delta,uint64_t tID);

void init_APIC_timer();

#endif //IOAPIC_H
