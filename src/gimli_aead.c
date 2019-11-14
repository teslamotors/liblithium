#include <lithium/gimli_aead.h>

#include "bytes.h"
#include "gimli_common.h"

static void load_words(uint32_t *s, const unsigned char *p, size_t nwords)
{
    for (size_t i = 0; i < nwords; ++i)
    {
        s[i] = bytes_to_u32(&p[4 * i]);
    }
}

static void gimli_aead_init(gimli_state *g,
                            const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                            const unsigned char k[GIMLI_AEAD_KEY_LEN],
                            const unsigned char *ad, size_t adlen)
{
    g->offset = 0;
    load_words(g->state, n, 4);
    load_words(&g->state[4], k, 8);
    gimli(g->state);
    gimli_absorb(g, ad, adlen);
    gimli_pad(g);
}

void gimli_aead_encrypt(unsigned char *c, unsigned char *t, size_t tlen,
                        const unsigned char *m, size_t mlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN])
{
    gimli_state g;
    gimli_aead_init(&g, n, k, ad, adlen);
    for (size_t i = 0; i < mlen; ++i)
    {
        gimli_absorb_byte(&g, m[i]);
        c[i] = gimli_squeeze_byte(&g);
        gimli_advance(&g);
    }
    gimli_pad(&g);
    gimli_squeeze(&g, t, tlen);
}

bool gimli_aead_decrypt(unsigned char *m, const unsigned char *c, size_t clen,
                        const unsigned char *t, size_t tlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN])
{
    gimli_state g;
    gimli_aead_init(&g, n, k, ad, adlen);
    for (size_t i = 0; i < clen; ++i)
    {
        m[i] = c[i] ^ gimli_squeeze_byte(&g);
        gimli_absorb_byte(&g, m[i]);
        gimli_advance(&g);
    }
    gimli_pad(&g);
    unsigned char result = 0;
    for (size_t i = 0; i < tlen; ++i)
    {
        result |= t[i] ^ gimli_squeeze_byte(&g);
        gimli_advance(&g);
    }
    result = (unsigned char)(((int32_t)result - 1) >> 16);
    for (size_t i = 0; i < clen; ++i)
    {
        m[i] &= result;
    }
    return result;
}
