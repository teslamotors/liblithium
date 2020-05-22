#ifndef LITHIUM_BYTES_H
#define LITHIUM_BYTES_H

#include <stdint.h>

uint32_t bytes_to_u32(const unsigned char *p);

void bytes_from_u32(unsigned char *p, uint32_t x);

#endif /* LITHIUM_BYTES_H */
