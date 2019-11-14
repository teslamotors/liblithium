#include "gimli_common.h"

#include <limits.h>
#include <string.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) &&             \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && (CHAR_BIT == 8)
#define GIMLI_LITTLE_ENDIAN 1
#else
#define GIMLI_LITTLE_ENDIAN 0
#endif

void gimli_absorb_byte(gimli_state *g, unsigned char x)
{
#if (GIMLI_LITTLE_ENDIAN)
    ((unsigned char *)g->state)[g->offset] ^= x;
#else
    g->state[g->offset / 4] ^= (uint32_t)x << ((g->offset % 4) * 8);
#endif
}

unsigned char gimli_squeeze_byte(const gimli_state *g)
{
#if (GIMLI_LITTLE_ENDIAN)
    return ((const unsigned char *)g->state)[g->offset];
#else
    return (g->state[g->offset / 4] >> ((g->offset % 4) * 8)) & 0xFFU;
#endif
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
    memset(g, 0, sizeof *g);
}

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        gimli_absorb_byte(g, m[i]);
        gimli_advance(g);
    }
}

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len)
{
    for (size_t i = 0; i < len; ++i)
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
