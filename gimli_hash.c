#include "gimli_hash.h"

#include "gimli.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static unsigned char gimli_byte_read(uint32_t state[static 12], size_t i)
{
    return (state[i / 4] >> (8 * (i % 4))) & 0xFFU;
}

void gimli_hash_init(gimli_hash_state *state)
{
    // === Initialize the state ===
    memset(state, 0, sizeof(*state));
}

void gimli_hash_update(gimli_hash_state *state, const unsigned char *input,
                       size_t len)
{
    // === Absorb all the input blocks ===
    while (len > 0)
    {
        size_t block_len = MIN(len, GIMLI_RATE - state->offset);
        for (size_t i = 0; i < block_len; i++)
            gimli_xor8(state->state, state->offset + i, input[i]);
        input += block_len;
        state->offset += block_len;
        len -= block_len;

        if (state->offset == GIMLI_RATE)
        {
            gimli(state->state);
            state->offset = 0;
        }
    }
}

void gimli_hash_final(gimli_hash_state *state, unsigned char *output,
                      size_t len)
{
    // === Do the padding and switch to the squeezing phase ===
    gimli_xor8(state->state, state->offset, 0x1F);
    // Add the second bit of padding
    gimli_xor8(state->state, GIMLI_RATE - 1, 0x80);
    // Switch to the squeezing phase
    gimli(state->state);

    // === Squeeze out all the output blocks ===
    while (len > 0)
    {
        state->offset = MIN(len, GIMLI_RATE);
        for (size_t i = 0; i < state->offset; ++i)
        {
            output[i] = gimli_byte_read(state->state, i);
        }
        output += state->offset;
        len -= state->offset;

        if (len > 0)
            gimli(state->state);
    }
}

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    gimli_hash_state state;
    gimli_hash_init(&state);
    gimli_hash_update(&state, input, input_len);
    gimli_hash_final(&state, output, output_len);
}
