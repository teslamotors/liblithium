#include <lithium/gimli_hash.h>

#include <string.h>

void gimli_hash_init(gimli_hash_state *g)
{
    memset(g, 0, sizeof *g);
}

static void advance(gimli_hash_state *g)
{
    ++g->offset;
    if (g->offset == GIMLI_RATE)
    {
        gimli(g->state);
        g->offset = 0;
    }
}

static void absorb_byte(gimli_hash_state *g, unsigned char x)
{
    g->state[g->offset / 4] ^= (uint32_t)x << ((g->offset % 4) * 8);
}

static unsigned char squeeze_byte(const gimli_hash_state *g)
{
    return (g->state[g->offset / 4] >> ((g->offset % 4) * 8)) & 0xFFU;
}

void gimli_hash_update(gimli_hash_state *g, const unsigned char *input,
                       size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        absorb_byte(g, input[i]);
        advance(g);
    }
}

void gimli_hash_final(gimli_hash_state *g, unsigned char *output, size_t len)
{
    absorb_byte(g, 0x01);
    g->state[GIMLI_WORDS - 1] ^= UINT32_C(0x01000000);
    g->offset = GIMLI_RATE - 1;
    for (size_t i = 0; i < len; ++i)
    {
        advance(g);
        output[i] = squeeze_byte(g);
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
