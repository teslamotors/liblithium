#pragma once

#include <stdint.h>

uint32_t bytes_to_u32(const unsigned char *p);

void bytes_from_u32(unsigned char *p, uint32_t x);
