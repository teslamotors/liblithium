#include <lithium/gimli.h>

static uint32_t rotate(uint32_t x, int n)
{
    if (n == 0)
    {
        return x;
    }
    return (x << n) | (x >> (32 - n));
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

        switch (round & 3)
        {
        case 0:
            // small swap: pattern s...s...s... etc.
            x = state[0];
            state[0] = state[1];
            state[1] = x;
            x = state[2];
            state[2] = state[3];
            state[3] = x;
            // add constant: pattern c...c...c... etc.
            state[0] ^= (0x9e377900 | round);
            break;

        case 2:
            // big swap: pattern ..S...S...S. etc.
            x = state[0];
            state[0] = state[2];
            state[2] = x;
            x = state[1];
            state[1] = state[3];
            state[3] = x;
            break;
        }
    }
}
