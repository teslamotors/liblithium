#include <lithium/gimli.h>

static unsigned char gimli_read8(const uint32_t state[static GIMLI_WORDS],
                                 size_t i)
{
    return (state[i / 4] >> (8 * (i % 4))) & 0xFFU;
}

static void gimli_write8(uint32_t state[static GIMLI_WORDS], size_t i,
                         unsigned char x)
{
    const size_t wi = i / 4;
    const int sh = (8 * (i % 4));
    state[wi] &= ~(UINT32_C(0xFF) << sh);
    state[wi] |= (uint32_t)x << sh;
}

void gimli_xor8(uint32_t state[static GIMLI_WORDS], size_t i, unsigned char x)
{
    state[i / 4] ^= (uint32_t)x << (8 * (i % 4));
}

static uint32_t rotate(uint32_t x, int bits)
{
    if (bits == 0)
        return x;
    return (x << bits) | (x >> (32 - bits));
}

void gimli(uint32_t *state)
{
    unsigned int round;
    unsigned int column;
    uint32_t x;
    uint32_t y;
    uint32_t z;

    for (round = 24; round > 0; --round)
    {
        for (column = 0; column < 4; ++column)
        {
            x = rotate(state[column], 24);
            y = rotate(state[4 + column], 9);
            z = state[8 + column];

            state[8 + column] = x ^ (z << 1) ^ ((y & z) << 2);
            state[4 + column] = y ^ x ^ ((x | z) << 1);
            state[column] = z ^ y ^ ((x & y) << 3);
        }

        if ((round & 3) == 0)
        { // small swap: pattern s...s...s... etc.
            x = state[0];
            state[0] = state[1];
            state[1] = x;
            x = state[2];
            state[2] = state[3];
            state[3] = x;
        }
        if ((round & 3) == 2)
        { // big swap: pattern ..S...S...S. etc.
            x = state[0];
            state[0] = state[2];
            state[2] = x;
            x = state[1];
            state[1] = state[3];
            state[3] = x;
        }

        if ((round & 3) == 0)
        { // add constant: pattern c...c...c... etc.
            state[0] ^= (0x9e377900 | round);
        }
    }
}
