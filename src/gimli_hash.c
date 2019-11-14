#include <lithium/gimli_hash.h>

#include "gimli_common.h"

#include <string.h>

void gimli_hash_init(gimli_hash_state *g)
{
    gimli_init(g);
}

void gimli_hash_update(gimli_hash_state *g, const unsigned char *m, size_t len)
{
    gimli_absorb(g, m, len);
}

void gimli_hash_final(gimli_hash_state *g, unsigned char *h, size_t len)
{
    gimli_pad(g);
    gimli_squeeze(g, h, len);
}

void gimli_hash(unsigned char *h, size_t hlen, const unsigned char *m,
                size_t mlen)
{
    gimli_state g;
    gimli_hash_init(&g);
    gimli_hash_update(&g, m, mlen);
    gimli_hash_final(&g, h, hlen);
}
