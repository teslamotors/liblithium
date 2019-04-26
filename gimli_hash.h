#pragma once

#include <stddef.h>

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len);
