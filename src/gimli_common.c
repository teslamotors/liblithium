/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gimli_common.h"

#include <lithium/watchdog.h>

#include "opt.h"

#include <string.h>

uint32_t gimli_load(const unsigned char *p)
{
#if (LITH_LITTLE_ENDIAN)
    uint32_t x;
    (void)memcpy(&x, p, sizeof x);
    return x;
#else
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
#endif
}

void gimli_store(unsigned char *p, uint32_t x)
{
#if (LITH_LITTLE_ENDIAN)
    (void)memcpy(p, &x, sizeof x);
#else
    p[0] = (unsigned char)(x & 0xFFU);
    p[1] = (unsigned char)((x >> 8) & 0xFFU);
    p[2] = (unsigned char)((x >> 16) & 0xFFU);
    p[3] = (unsigned char)((x >> 24) & 0xFFU);
#endif
}

void gimli_absorb_byte(gimli_state *g, unsigned char x)
{
#if (LITH_LITTLE_ENDIAN)
    ((unsigned char *)g->state)[g->offset] ^= x;
#else
    const int sh = (g->offset % 4) * 8;
    g->state[g->offset / 4] ^= (uint32_t)x << sh;
#endif
}

unsigned char gimli_squeeze_byte(const gimli_state *g)
{
#if (LITH_LITTLE_ENDIAN)
    return ((const unsigned char *)g->state)[g->offset];
#else
    const int sh = (g->offset % 4) * 8;
    return (unsigned char)((g->state[g->offset / 4] >> sh) & 0xFFU);
#endif
}

void gimli_advance(gimli_state *g)
{
    ++g->offset;
    if (g->offset == GIMLI_RATE)
    {
#if (LITH_ENABLE_WATCHDOG)
        lith_watchdog_pet();
#endif
        gimli(g->state);
        g->offset = 0;
    }
}

void gimli_init(gimli_state *g)
{
    (void)memset(g, 0, sizeof *g);
}

static void absorb(gimli_state *g, const unsigned char *m, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        gimli_absorb_byte(g, m[i]);
        gimli_advance(g);
    }
}

void gimli_absorb(gimli_state *g, const unsigned char *m, size_t len)
{
#if (LITH_SPONGE_WORDS)
    const size_t first_block_len = (GIMLI_RATE - g->offset) % GIMLI_RATE;
    if (len >= GIMLI_RATE + first_block_len)
    {
        absorb(g, m, first_block_len);
        m += first_block_len;
        len -= first_block_len;
        do
        {
#if (LITH_SPONGE_VECTORS)
            typedef uint32_t block_t
                __attribute__((vector_size(16), aligned(1)));
            *(block_t *)g->state ^= *(const block_t *)m;
            m += GIMLI_RATE;
#else
            size_t i;
            for (i = 0; i < GIMLI_RATE / 4; ++i)
            {
                g->state[i] ^= gimli_load(m);
                m += 4;
            }
#endif
            gimli(g->state);
            len -= GIMLI_RATE;
        } while (len >= GIMLI_RATE);
    }
#endif
    absorb(g, m, len);
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
