#ifndef LITHIUM_GIMLI_H
#define LITHIUM_GIMLI_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/* cffi:begin */

#define GIMLI_WORDS 12U

void gimli(uint32_t state[GIMLI_WORDS]);

/* cffi:end */

#endif /* LITHIUM_GIMLI_H */
