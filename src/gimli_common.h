#ifndef LITHIUM_GIMLI_COMMON_H
#define LITHIUM_GIMLI_COMMON_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli_state.h>

#include <stdint.h>

void gimli_absorb_byte(uint32_t *state, unsigned offset, unsigned char x);

unsigned char gimli_squeeze_byte(const uint32_t *state, unsigned offset);

void gimli_advance(uint32_t *g, unsigned *offset);

void gimli_pad(uint32_t *state, unsigned offset);

uint32_t gimli_load(const unsigned char *p);

void gimli_store(unsigned char *p, uint32_t x);

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len);

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len);

#define GIMLI_RATE 16U

#endif /* LITHIUM_GIMLI_COMMON_H */
