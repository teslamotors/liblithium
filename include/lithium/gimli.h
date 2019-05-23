#pragma once

#include <stddef.h>
#include <stdint.h>

#define GIMLI_WORDS 12
#define GIMLI_RATE 16

void gimli(uint32_t state[static GIMLI_WORDS]);

void gimli_xor8(uint32_t state[static GIMLI_WORDS], size_t i, unsigned char x);

unsigned char gimli_read8(const uint32_t state[static GIMLI_WORDS], size_t i);
