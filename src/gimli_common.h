#pragma once

#include <lithium/gimli_state.h>

void gimli_absorb_byte(gimli_state *g, unsigned char x);

unsigned char gimli_squeeze_byte(const gimli_state *g);

void gimli_advance(gimli_state *g);

void gimli_init(gimli_state *g);

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len);

void gimli_pad(gimli_state *g);

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len);
