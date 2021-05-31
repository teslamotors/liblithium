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
#define shuffleb shuffle
#else /* Use the gcc shuffle builtin. */
#define shuffle(x, ...) (__builtin_shuffle(x, (uint32x4_t){__VA_ARGS__}))
#define shuffleb(x, ...) (__builtin_shuffle(x, (uint8x16_t){__VA_ARGS__}))
#endif

static uint32x4_t rol24(uint32x4_t x)
{
    /*
     * Rotate left by 24 bits can be achieved more efficiently with a shuffle
     * if there is no vector rotate.
     * vpshufb is part of SSSE3, and vprold is part of AVX512VL.
     * Neon doesn't have a vector rotate, but does have shuffles.
     */
#if (defined(__SSSE3__) && !defined(__AVX512VL__)) || defined(__ARM_NEON)
    uint8x16_t xb = (uint8x16_t)x;
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    xb = shuffleb(xb, 1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12);
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    xb = shuffleb(xb, 3, 0, 1, 2, 7, 4, 5, 6, 11, 8, 9, 10, 15, 12, 13, 14);
#else
#error "Can't determine which byte order to use for byte shuffle rol24."
#endif
    return (uint32x4_t)xb;
#else
    return rol(x, 24);
#endif
}

void gimli(uint32_t *state)
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
        }
    }
    s[0] = x;
    s[1] = y;
    s[2] = z;
}

#else /* !LITH_VECTORIZE */

static void swap(uint32_t *x, int i, int j)
{
    const uint32_t tmp = x[i];
    x[i] = x[j];
    x[j] = tmp;
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
        switch (round & 3)
        {
        case 0:
            /* small swap: pattern s...s...s... etc. */
            swap(state, 0, 1);
            swap(state, 2, 3);
            /* add constant: pattern c...c...c... etc. */
            state[0] ^= coeff(round);
            break;
        case 2:
            /* big swap: pattern ..S...S...S. etc. */
            swap(state, 0, 2);
            swap(state, 1, 3);
            break;
        }
    }
}

#endif /* LITH_VECTORIZE */
