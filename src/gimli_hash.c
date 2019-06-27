#include <lithium/gimli_hash.h>

#include <limits.h>
#include <string.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) &&             \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && (CHAR_BIT == 8)
#define LITH_LITTLE_ENDIAN 1
#elif
#define LITH_LITTLE_ENDIAN 0
#endif

unsigned char gimli_read8(const uint32_t *state, size_t i)
{
#if (LITH_LITTLE_ENDIAN)
    return ((const unsigned char *)state)[i];
#else
    return (unsigned char)(state[i / 4] >> (8 * (i % 4))) & 0xFFU;
#endif
}

void gimli_xor8(uint32_t *state, size_t i, unsigned char x)
{
#if (LITH_LITTLE_ENDIAN)
    ((unsigned char *)state)[i] ^= x;
#else
    state[i / 4] ^= (uint32_t)x << (8 * (i % 4));
#endif
}

void gimli_hash_init(gimli_hash_state *g)
{
    memset(g, 0, sizeof(*g));
}

void gimli_hash_update(gimli_hash_state *g, const unsigned char *input,
                       size_t len)
{
    size_t offset = g->offset;
    for (size_t i = 0; i < len; ++i)
    {
        gimli_xor8(g->state, offset, input[i]);
        ++offset;
        if (offset == GIMLI_RATE)
        {
            gimli(g->state);
            offset = 0;
        }
    }
    g->offset = offset;
}

void gimli_hash_final(gimli_hash_state *g, unsigned char *output, size_t len)
{
    // Apply padding.
    gimli_xor8(g->state, g->offset, 0x1F);
    gimli_xor8(g->state, GIMLI_RATE - 1, 0x80);

    // Switch to the squeezing phase.
    size_t offset = GIMLI_RATE;
    for (size_t i = 0; i < len; ++i)
    {
        if (offset == GIMLI_RATE)
        {
            gimli(g->state);
            offset = 0;
        }
        output[i] = gimli_read8(g->state, offset);
        ++offset;
    }
}

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    gimli_hash_state g;
    gimli_hash_init(&g);
    gimli_hash_update(&g, input, input_len);
    gimli_hash_final(&g, output, output_len);
}
