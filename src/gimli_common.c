#include "gimli_common.h"

#include "lith_endian.h"

#include <string.h>

void gimli_absorb_byte(gimli_state *g, unsigned char x)
{
    if (LITH_LITTLE_ENDIAN)
    {
        ((unsigned char *)g->state)[g->offset] ^= x;
    }
    else
    {
        g->state[g->offset / 4] ^= (uint32_t)x << ((g->offset % 4) * 8);
    }
}

unsigned char gimli_squeeze_byte(const gimli_state *g)
{
    if (LITH_LITTLE_ENDIAN)
    {
        return ((const unsigned char *)g->state)[g->offset];
    }
    else
    {
        return (g->state[g->offset / 4] >> ((g->offset % 4) * 8)) & 0xFFU;
    }
}

#define GIMLI_RATE 16

void gimli_advance(gimli_state *g)
{
    ++g->offset;
    if (g->offset == GIMLI_RATE)
    {
        gimli(g->state);
        g->offset = 0;
    }
}

void gimli_init(gimli_state *g)
{
    (void)memset(g, 0, sizeof *g);
}

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        gimli_absorb_byte(g, m[i]);
        gimli_advance(g);
    }
}

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        h[i] = gimli_squeeze_byte(g);
        gimli_advance(g);
    }
}

void gimli_pad(gimli_state *g)
{
    gimli_absorb_byte(g, 0x01);
    g->state[GIMLI_WORDS - 1] ^= UINT32_C(0x01000000);
    gimli(g->state);
    g->offset = 0;
}
