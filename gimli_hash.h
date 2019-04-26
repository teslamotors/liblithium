#include <stdint.h>
#include <string.h>
#include "gimli.h"

#ifndef _GIMLI_HASH_h_
#define _GIMLI_HASH_h_

extern void Gimli_hash(const uint8_t *input, uint64_t inputByteLen, uint8_t *output, uint64_t outputByteLen);

#endif