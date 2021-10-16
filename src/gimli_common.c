/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gimli_common.h"

#include <lithium/gimli.h>
#include <lithium/watchdog.h>

#include "opt.h"

#include <string.h>

uint32_t gimli_load(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

void gimli_store(unsigned char *p, uint32_t x)
{
    p[0] = (unsigned char)(x & 0xFFU);
    p[1] = (unsigned char)((x >> 8) & 0xFFU);
    p[2] = (unsigned char)((x >> 16) & 0xFFU);
    p[3] = (unsigned char)((x >> 24) & 0xFFU);
}

void gimli_absorb_byte(uint32_t *state, size_t offset, unsigned char x)
{
#if (LITH_LITTLE_ENDIAN)
    ((unsigned char *)state)[offset] ^= x;
#else
    const int sh = (offset % 4) * 8;
    state[offset / 4] ^= (uint32_t)x << sh;
#endif
}

unsigned char gimli_squeeze_byte(const uint32_t *state, size_t offset)
{
#if (LITH_LITTLE_ENDIAN)
    return ((const unsigned char *)state)[offset];
#else
    const int sh = (offset % 4) * 8;
    return (unsigned char)((state[offset / 4] >> sh) & 0xFFU);
#endif
}

void gimli_advance(uint32_t *state, size_t *offset)
{
    ++*offset;
    if (*offset == GIMLI_RATE)
    {
#if (LITH_ENABLE_WATCHDOG)
        lith_watchdog_pet();
#endif
        gimli(state);
        *offset = 0;
    }
}

static void absorb(uint32_t *state, size_t *offset, const unsigned char *m,
                   size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        gimli_absorb_byte(state, *offset, m[i]);
        gimli_advance(state, offset);
    }
}

void gimli_absorb(gimli_state *g, const unsigned char *m,
                  size_t len)
{
    size_t offset = g->offset;
#if (LITH_SPONGE_WORDS)
    const size_t first_block_len = (GIMLI_RATE - offset) % GIMLI_RATE;
    if (len >= GIMLI_RATE + first_block_len)
    {
        absorb(g->state, &offset, m, first_block_len);
        m += first_block_len;
        len -= first_block_len;
        do
        {
#if (LITH_SPONGE_VECTORS)
            *(block *)g->state ^= *(const block *)m;
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
    absorb(g->state, &offset, m, len);
    g->offset = offset;
}

void gimli_squeeze(gimli_state *g, unsigned char *h, size_t len)
{
    size_t i, offset = GIMLI_RATE - 1;
    for (i = 0; i < len; ++i)
    {
        gimli_advance(g->state, &offset);
        h[i] = gimli_squeeze_byte(g->state, offset);
    }
    g->offset = offset;
}

void gimli_pad(uint32_t *state, size_t offset)
{
    gimli_absorb_byte(state, offset, 0x01);
    state[GIMLI_WORDS - 1] ^= UINT32_C(0x01000000);
}
