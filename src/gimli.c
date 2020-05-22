#include <lithium/gimli.h>

static uint32_t rol(uint32_t x, int n)
{
    return (x << (n % 32)) | (x >> ((32 - n) % 32));
}

static void swap(uint32_t *x, uint32_t *y)
{
    const uint32_t tmp = *x;
    *x = *y;
    *y = tmp;
}

void gimli(uint32_t *state)
{
    int round;
    for (round = 24; round > 0; --round)
    {
        int column;
        for (column = 0; column < 4; ++column)
        {
            const uint32_t x = rol(state[column], 24);
            const uint32_t y = rol(state[column + 4], 9);
            const uint32_t z = state[column + 8];
            state[column + 8] = x ^ (z << 1) ^ ((y & z) << 2);
            state[column + 4] = y ^ x ^ ((x | z) << 1);
            state[column] = z ^ y ^ ((x & y) << 3);
        }
        switch (round % 4)
        {
        case 0:
            /* small swap: pattern s...s...s... etc. */
            swap(&state[0], &state[1]);
            swap(&state[2], &state[3]);
            /* add constant: pattern c...c...c... etc. */
            state[0] ^= UINT32_C(0x9E377900) | (uint32_t)round;
            break;
        case 2:
            /* big swap: pattern ..S...S...S. etc. */
            swap(&state[0], &state[2]);
            swap(&state[1], &state[3]);
            break;
        }
    }
}
