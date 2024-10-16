/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli.h>

#include "opt.h"

static uint32_t coeff(int round)
{
    return UINT32_C(0x9E377900) | (uint32_t)round;
}

#define rol(x, n) (((x) << ((n) % 32)) | ((x) >> ((32 - (n)) % 32)))

#if (LITH_VECTORIZE)

typedef uint32_t uint32x4_t __attribute__((vector_size(16), aligned(4)));
typedef uint8_t uint8x16_t __attribute__((vector_size(16), aligned(4)));

#if defined(__clang__)
#define shuffle(x, ...) (__builtin_shufflevector(x, x, __VA_ARGS__))
#else /* Use the gcc shuffle builtin. */
#define shuffle(x, ...) (__builtin_shuffle(x, (__typeof__(x)){__VA_ARGS__}))
#endif

static uint32x4_t rol24(uint32x4_t x)
{
#if (LITH_SHUFFLE_ROL24)
    uint8x16_t xb = (uint8x16_t)x;
#if (LITH_LITTLE_ENDIAN)
    xb = shuffle(xb, 1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12);
#elif (LITH_BIG_ENDIAN)
    xb = shuffle(xb, 3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14);
#else
#error "no byte order to use for byte shuffle implementation of rol24"
#endif
    return (uint32x4_t)xb;
#else
    return rol(x, 24);
#endif
}

void gimli(uint32_t state[GIMLI_WORDS])
{
    uint32x4_t *s = (uint32x4_t *)state;
    uint32x4_t x = s[0];
    uint32x4_t y = s[1];
    uint32x4_t z = s[2];
    int round;
    for (round = 24; round > 0; --round)
    {
        uint32x4_t newy, newz;
        x = rol24(x);
        y = rol(y, 9);
        newz = x ^ (z << 1) ^ ((y & z) << 2);
        newy = y ^ x ^ ((x | z) << 1);
        x = z ^ y ^ ((x & y) << 3);
        y = newy;
        z = newz;
        switch (round & 3)
        {
        case 0:
            /* small swap: pattern s...s...s... etc. */
            x = shuffle(x, 1, 0, 3, 2);
            /* add constant: pattern c...c...c... etc. */
            x ^= (uint32x4_t){coeff(round)};
            break;
        case 2:
            /* big swap: pattern ..S...S...S. etc. */
            x = shuffle(x, 2, 3, 0, 1);
            break;
        default:
            break;
        }
    }
    s[0] = x;
    s[1] = y;
    s[2] = z;
}

#else /* !LITH_VECTORIZE */

void gimli(uint32_t state[GIMLI_WORDS])
{
    int round;
    for (round = 24; round > 0; --round)
    {
        uint32_t tmp;
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
        switch (round & 3)
        {
        case 0:
            /* small swap: pattern s...s...s... etc. */
            /* add constant: pattern c...c...c... etc. */
            tmp = state[0];
            state[0] = state[1] ^ coeff(round);
            state[1] = tmp;
            tmp = state[2];
            state[2] = state[3];
            state[3] = tmp;
            break;
        case 2:
            /* big swap: pattern ..S...S...S. etc. */
            tmp = state[0];
            state[0] = state[2];
            state[2] = tmp;
            tmp = state[1];
            state[1] = state[3];
            state[3] = tmp;
            break;
        default:
            break;
        }
    }
}

#endif /* LITH_VECTORIZE */
