#ifndef LITHIUM_GIMLI_COMMON_H
#define LITHIUM_GIMLI_COMMON_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli_state.h>

#include <stdint.h>

void gimli_absorb_byte(gimli_state *g, unsigned char x);

unsigned char gimli_squeeze_byte(const gimli_state *g);

void gimli_advance(gimli_state *g);

void gimli_init(gimli_state *g);

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len);

void gimli_pad(gimli_state *g);

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len);

uint32_t gimli_load(const unsigned char *p);

void gimli_store(unsigned char *p, uint32_t x);

#define GIMLI_RATE 16

#endif /* LITHIUM_GIMLI_COMMON_H */
