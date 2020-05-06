#include <lithium/gimli_aead.h>

#include "bytes.h"
#include "gimli_common.h"

static void load_words(uint32_t *s, const unsigned char *p, size_t nwords)
{
    size_t i;
    for (i = 0; i < nwords; ++i)
    {
        s[i] = bytes_to_u32(&p[4 * i]);
    }
}

void gimli_aead_init(gimli_state *g,
                     const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                     const unsigned char k[GIMLI_AEAD_KEY_LEN])
{
    g->offset = 0;
    load_words(g->state, n, 4);
    load_words(&g->state[4], k, 8);
    gimli(g->state);
}

void gimli_aead_update_ad(gimli_state *g, const unsigned char *ad, size_t len)
{
    gimli_absorb(g, ad, len);
}

void gimli_aead_final_ad(gimli_state *g)
{
    gimli_pad(g);
}

void gimli_aead_encrypt_update(gimli_state *g, unsigned char *c,
                               const unsigned char *m, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        gimli_absorb_byte(g, m[i]);
        c[i] = gimli_squeeze_byte(g);
        gimli_advance(g);
    }
}

void gimli_aead_encrypt_final(gimli_state *g, unsigned char *t, size_t len)
{
    gimli_pad(g);
    gimli_squeeze(g, t, len);
}

void gimli_aead_decrypt_update(gimli_state *g, unsigned char *m,
                               const unsigned char *c, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
    {
        m[i] = c[i] ^ gimli_squeeze_byte(g);
        gimli_absorb_byte(g, m[i]);
        gimli_advance(g);
    }
}

bool gimli_aead_decrypt_final(gimli_state *g, const unsigned char *t,
                              size_t len)
{
    unsigned char mismatch = 0;
    size_t i;
    gimli_pad(g);
    for (i = 0; i < len; ++i)
    {
        mismatch |= t[i] ^ gimli_squeeze_byte(g);
        gimli_advance(g);
    }
    return !mismatch;
}

void gimli_aead_encrypt(unsigned char *c, unsigned char *t, size_t tlen,
                        const unsigned char *m, size_t mlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN])
{
    gimli_state g;
    gimli_aead_init(&g, n, k);
    gimli_aead_update_ad(&g, ad, adlen);
    gimli_aead_final_ad(&g);
    gimli_aead_encrypt_update(&g, c, m, mlen);
    gimli_aead_encrypt_final(&g, t, tlen);
}

bool gimli_aead_decrypt(unsigned char *m, const unsigned char *c, size_t clen,
                        const unsigned char *t, size_t tlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN])
{
    bool success;
    unsigned char mask;
    size_t i;
    gimli_state g;
    gimli_aead_init(&g, n, k);
    gimli_aead_update_ad(&g, ad, adlen);
    gimli_aead_final_ad(&g);
    gimli_aead_decrypt_update(&g, m, c, clen);
    success = gimli_aead_decrypt_final(&g, t, tlen);
    mask = (unsigned char)~(((uint32_t)success - 1) >> 16);
    for (i = 0; i < clen; ++i)
    {
        m[i] &= mask;
    }
    return success;
}
