//
// Created by igel on 04.09.24.
//

#ifndef MSR_H
#define MSR_H

#include <stdint.h>

uint64_t read_msr(uint32_t msr);

void write_msr(uint32_t msr, uint64_t value);

#endif //MSR_H
