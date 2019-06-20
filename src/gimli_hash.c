#include <lithium/gimli_hash.h>

#include <limits.h>
#include <string.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) &&             \
    (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && (CHAR_BIT == 8)
#define LITH_LITTLE_ENDIAN 1
#else
#define LITH_LITTLE_ENDIAN 0
#endif

#if defined(__OPTIMIZE_SIZE__) || defined(_OPTIMIZE_FOR_SPACE)
#define LITH_SMALL 1
#else
#define LITH_SMALL 0
#endif

#define GIMLI_RATE 16

static unsigned char gimli_read8(const uint32_t *state, size_t i)
{
    if (LITH_LITTLE_ENDIAN)
    {
        return ((const unsigned char *)state)[i];
    }
    else
    {
        return (unsigned char)((state[i / 4] >> (8 * (i % 4))) & 0xFFU);
    }
}

static void gimli_xor8(uint32_t *state, size_t i, unsigned char x)
{
    if (LITH_LITTLE_ENDIAN)
    {
        ((unsigned char *)state)[i] ^= x;
    }
    else
    {
        state[i / 4] ^= (uint32_t)x << (8 * (i % 4));
    }
}

void gimli_hash_init(gimli_hash_state *g)
{
    memset(g, 0, sizeof(*g));
}

static void update(gimli_hash_state *g, const unsigned char *input, size_t len)
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

#define GIMLI_RATE_WORDS (GIMLI_RATE / 4)

void gimli_hash_update(gimli_hash_state *g, const unsigned char *input,
                       size_t len)
{
    if (LITH_LITTLE_ENDIAN && !LITH_SMALL)
    {
        const size_t first_block_len = (GIMLI_RATE - g->offset) % GIMLI_RATE;
        if (len >= GIMLI_RATE + first_block_len)
        {
            update(g, input, first_block_len);
            input += first_block_len;
            len -= first_block_len;
            do
            {
                uint32_t block[GIMLI_RATE_WORDS];
                memcpy(block, input, GIMLI_RATE);
                for (size_t i = 0; i < GIMLI_RATE_WORDS; ++i)
                {
                    g->state[i] ^= block[i];
                }
                gimli(g->state);
                input += GIMLI_RATE;
                len -= GIMLI_RATE;
            } while (len >= GIMLI_RATE);
        }
    }
    update(g, input, len);
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
