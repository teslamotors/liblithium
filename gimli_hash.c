#include "gimli_hash.h"

#include "gimli.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define GIMLI_HASH_RATE 16

static void gimli_byte_xor(uint32_t state[static 12], size_t i, unsigned char x)
{
    state[i / 4] ^= (uint32_t)x << (8 * (i % 4));
}

static unsigned char gimli_byte_read(uint32_t state[static 12], size_t i)
{
    return (state[i / 4] >> (8 * (i % 4))) & 0xFFU;
}

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    uint32_t state[12];
    size_t block_len = 0;

    // === Initialize the state ===
    memset(state, 0, sizeof(state));

    // === Absorb all the input blocks ===
    while (input_len > 0)
    {
        block_len = MIN(input_len, GIMLI_HASH_RATE);
        for (size_t i = 0; i < block_len; i++)
            gimli_byte_xor(state, i, input[i]);
        input += block_len;
        input_len -= block_len;

        if (block_len == GIMLI_HASH_RATE)
        {
            gimli(state);
            block_len = 0;
        }
    }

    // === Do the padding and switch to the squeezing phase ===
    gimli_byte_xor(state, block_len, 0x1F);
    // Add the second bit of padding
    gimli_byte_xor(state, GIMLI_HASH_RATE - 1, 0x80);
    // Switch to the squeezing phase
    gimli(state);

    // === Squeeze out all the output blocks ===
    while (output_len > 0)
    {
        block_len = MIN(output_len, GIMLI_HASH_RATE);
        for (size_t i = 0; i < block_len; ++i)
        {
            output[i] = gimli_byte_read(state, i);
        }
        output += block_len;
        output_len -= block_len;

        if (output_len > 0)
            gimli(state);
    }
}
