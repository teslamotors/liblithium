#ifndef LITHIUM_GIMLI_STATE_H
#define LITHIUM_GIMLI_STATE_H

#include <lithium/gimli.h>

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint32_t state[GIMLI_WORDS];
    size_t offset;
} gimli_state;

#endif /* LITHIUM_GIMLI_STATE_H */
