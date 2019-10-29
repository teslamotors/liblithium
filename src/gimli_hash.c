#include <lithium/gimli_hash.h>

#include "bytes.h"

#include <limits.h>
#include <string.h>

#define GIMLI_RATE_WORDS (GIMLI_RATE / 4)

void gimli_hash_init(gimli_hash_state *g)
{
    memset(g, 0, sizeof *g);
}

static void absorb(uint32_t *state, const unsigned char m[GIMLI_RATE])
{
    for (size_t i = 0; i < GIMLI_RATE_WORDS; ++i)
    {
        state[i] ^= bytes_to_u32(&m[4 * i]);
    }
}

static void squeeze(uint32_t *state, unsigned char h[GIMLI_RATE])
{
    for (size_t i = 0; i < GIMLI_RATE_WORDS; ++i)
    {
        bytes_from_u32(&h[4 * i], state[i]);
    }
}

void gimli_hash_update(gimli_hash_state *g, const unsigned char *input,
                       size_t len)
{
    size_t offset = g->offset;
    for (size_t i = 0; i < len; ++i)
    {
        g->buf[offset] = input[i];
        ++offset;
        if (offset == GIMLI_RATE)
        {
            absorb(g->state, g->buf);
            gimli(g->state);
            offset = 0;
        }
    }
    g->offset = offset;
}

void gimli_hash_final(gimli_hash_state *g, unsigned char *output, size_t len)
{
    // Apply padding.
    g->buf[g->offset] = 0x01;
    for (size_t i = g->offset + 1; i < GIMLI_RATE; ++i)
    {
        g->buf[i] = 0;
    }
    g->state[GIMLI_WORDS - 1] ^= UINT32_C(0x01000000);
    absorb(g->state, g->buf);

    // Switch to the squeezing phase.
    size_t offset = GIMLI_RATE;
    for (size_t i = 0; i < len; ++i)
    {
        if (offset == GIMLI_RATE)
        {
            gimli(g->state);
            squeeze(g->state, g->buf);
            offset = 0;
        }
        output[i] = g->buf[offset];
        ++offset;
    }
}

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    gimli_hash_state g;
    gimli_hash_init(&g);
    gimli_hash_update(&g, input, input_len);
    gimli_hash_final(&g, output, output_len);
}
