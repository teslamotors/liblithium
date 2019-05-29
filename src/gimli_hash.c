#include <lithium/gimli_hash.h>

#include <lithium/gimli.h>

#include <string.h>

unsigned char gimli_read8(const uint32_t *state, size_t i)
{
    return (state[i / 4] >> (8 * (i % 4))) & 0xFFU;
}

void gimli_xor8(uint32_t *state, size_t i, unsigned char x)
{
    state[i / 4] ^= (uint32_t)x << (8 * (i % 4));
}

void gimli_hash_init(gimli_hash_state *g)
{
    memset(g, 0, sizeof(*g));
}

void gimli_hash_update(gimli_hash_state *g, const unsigned char *input,
                       size_t len)
{
    size_t offset = g->offset;
    for (size_t i = 0; i < len; ++i)
    {
        gimli_xor8(g->state, offset, input[i]);
        ++offset;
        if (offset == GIMLI_RATE)
        {
            gimli(g->state);
            offset = 0;
        }
    }
    g->offset = offset;
}

void gimli_hash_final(gimli_hash_state *g, unsigned char *output,
                      size_t len)
{
    // Apply padding.
    gimli_xor8(g->state, g->offset, 0x1F);
    gimli_xor8(g->state, GIMLI_RATE - 1, 0x80);

    // Switch to the squeezing phase.
    for (size_t i = 0; i < len; ++i)
    {
        if (i % GIMLI_RATE == 0)
        {
            gimli(g->state);
        }
        output[i] = gimli_read8(g->state, i % GIMLI_RATE);
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
