#include "gimli_hash.h"

#include "gimli.h"

#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define rateInBytes 16

void Gimli_hash(const uint8_t *input,
                uint64_t inputByteLen,
                uint8_t *output,
                uint64_t outputByteLen)
{
    uint32_t state[12];
    uint8_t* state_8 = (uint8_t*)state;
    uint64_t blockSize = 0;
    uint64_t i;

    // === Initialize the state ===
    memset(state, 0, sizeof(state));

    // === Absorb all the input blocks ===
    while(inputByteLen > 0) {
        blockSize = MIN(inputByteLen, rateInBytes);
        for(i=0; i<blockSize; i++)
            state_8[i] ^= input[i];
        input += blockSize;
        inputByteLen -= blockSize;

        if (blockSize == rateInBytes) {
            gimli(state);
            blockSize = 0;
        }
    }

    // === Do the padding and switch to the squeezing phase ===
    state_8[blockSize] ^= 0x1F;
    // Add the second bit of padding
    state_8[rateInBytes-1] ^= 0x80;
    // Switch to the squeezing phase
    gimli(state);

    // === Squeeze out all the output blocks ===
    while(outputByteLen > 0) {
        blockSize = MIN(outputByteLen, rateInBytes);
        memcpy(output, state, blockSize);
        output += blockSize;
        outputByteLen -= blockSize;

        if (outputByteLen > 0)
            gimli(state);
    }
}
