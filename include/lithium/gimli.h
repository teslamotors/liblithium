#pragma once

#include <stdint.h>

#define GIMLI_WORDS 12

void gimli(uint32_t state[static GIMLI_WORDS]);
