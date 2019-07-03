#include <lithium/gimli.h>

static uint32_t rotate(uint32_t x, int n)
{
    if (n == 0)
    {
        return x;
    }
    return (x << n) | (x >> (32 - n));
}

static void swap(uint32_t *x, uint32_t *y)
{
    const uint32_t tmp = *x;
    *x = *y;
    *y = tmp;
}

void gimli(uint32_t *state)
{
    for (int round = 24; round > 0; --round)
    {
        for (int column = 0; column < 4; ++column)
        {
            const uint32_t x = rotate(state[column], 24);
            const uint32_t y = rotate(state[column + 4], 9);
            const uint32_t z = state[column + 8];

            state[column + 8] = x ^ (z << 1) ^ ((y & z) << 2);
            state[column + 4] = y ^ x ^ ((x | z) << 1);
            state[column] = z ^ y ^ ((x & y) << 3);
        }

        switch (round & 3)
        {
        case 0:
            // small swap: pattern s...s...s... etc.
            swap(&state[0], &state[1]);
            swap(&state[2], &state[3]);
            // add constant: pattern c...c...c... etc.
            state[0] ^= UINT32_C(0x9e377900) | (uint32_t)round;
            break;

        case 2:
            // big swap: pattern ..S...S...S. etc.
            swap(&state[0], &state[2]);
            swap(&state[1], &state[3]);
            break;
        }
    }
}
