#include "gimli_hash.h"

#include "gimli.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define rateInBytes 16

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    uint32_t state[12];
    uint8_t *state_8 = (uint8_t *)state;
    uint64_t blockSize = 0;
    uint64_t i;

    // === Initialize the state ===
    memset(state, 0, sizeof(state));

    // === Absorb all the input blocks ===
    while (input_len > 0)
    {
        blockSize = MIN(input_len, rateInBytes);
        for (i = 0; i < blockSize; i++)
            state_8[i] ^= input[i];
        input += blockSize;
        input_len -= blockSize;

        if (blockSize == rateInBytes)
        {
            gimli(state);
            blockSize = 0;
        }
    }

    // === Do the padding and switch to the squeezing phase ===
    state_8[blockSize] ^= 0x1F;
    // Add the second bit of padding
    state_8[rateInBytes - 1] ^= 0x80;
    // Switch to the squeezing phase
    gimli(state);

    // === Squeeze out all the output blocks ===
    while (output_len > 0)
    {
        blockSize = MIN(output_len, rateInBytes);
        memcpy(output, state, blockSize);
        output += blockSize;
        output_len -= blockSize;

        if (output_len > 0)
            gimli(state);
    }
}
