#ifndef LITHIUM_GIMLI_STATE_H
#define LITHIUM_GIMLI_STATE_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli.h>

#include <stddef.h>
#include <stdint.h>

/* cffi:begin */

typedef struct
{
    uint32_t state[GIMLI_WORDS];
    unsigned offset;
} gimli_state;

/* cffi:end */

#endif /* LITHIUM_GIMLI_STATE_H */
