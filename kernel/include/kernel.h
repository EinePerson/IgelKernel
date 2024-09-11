#pragma once

#include <stdint.h>

struct limine_kernel_address_response* get_addresses();
uint64_t get_kernel_virt_offset();

///Hold and catch fire, hangs the computer
__attribute__((noreturn))
void hcf();