#include "hydrogen.h"

#include <lithium/gimli_hash.h>
#include <lithium/x25519.h>

#include <stdint.h>
#include <string.h>

#define HASH_PAD_LEN 16

void hydro_hash_keygen(uint8_t key[hydro_hash_KEYBYTES])
{
    hydro_random_buf(key, hydro_hash_KEYBYTES);
}

int hydro_hash_init(hydro_hash_state *state,
                    const char ctx[hydro_hash_CONTEXTBYTES],
                    const uint8_t key[hydro_hash_KEYBYTES])
{
    gimli_hash_init(state);
    static const unsigned char prefix[] = {4, 'k', 'm', 'a', 'c', 8};
    static const unsigned char pad[HASH_PAD_LEN] = {0};
    gimli_hash_update(state, prefix, sizeof prefix);
    gimli_hash_update(state, ctx ? (const unsigned char *)ctx : pad,
                      hydro_hash_CONTEXTBYTES);
    gimli_hash_update(state, pad, 2);
    if (key != NULL)
    {
        unsigned char key_len = hydro_hash_KEYBYTES;
        gimli_hash_update(state, &key_len, sizeof key_len);
        gimli_hash_update(state, key, hydro_hash_KEYBYTES);
        gimli_hash_update(state, pad, HASH_PAD_LEN - 1);
    }
    else
    {
        gimli_hash_update(state, pad, HASH_PAD_LEN);
    }
    return 0;
}

int hydro_hash_update(hydro_hash_state *state, const void *in_, size_t in_len)
{
    gimli_hash_update(state, in_, in_len);
    return 0;
}

int hydro_hash_final(hydro_hash_state *state, uint8_t *out, size_t out_len)
{
    if (out_len < hydro_hash_BYTES_MIN || out_len > hydro_hash_BYTES_MAX)
    {
        return -1;
    }
    const unsigned char hash_len_len = out_len > 0xFFU ? 2U : 1U;
    const unsigned char suffix[4] = {
        [0] = hash_len_len,
        [1] = out_len & 0xFFU,
        [2] = (out_len >> 8) & 0xFFU,
        [3] = 0x00U,
    };
    gimli_hash_update(state, suffix, hash_len_len + 2U);
    gimli_hash_final(state, out, out_len);
    return 0;
}

int hydro_hash_hash(uint8_t *out, size_t out_len, const void *in_,
                    size_t in_len, const char ctx[hydro_hash_CONTEXTBYTES],
                    const uint8_t key[hydro_hash_KEYBYTES])
{
    hydro_hash_state st;
    if (hydro_hash_init(&st, ctx, key) != 0 ||
        hydro_hash_update(&st, in_, in_len) != 0 ||
        hydro_hash_final(&st, out, out_len) != 0)
    {
        return -1;
    }
    return 0;
}

#define hydro_sign_CHALLENGEBYTES 32
#define hydro_sign_NONCEBYTES 32
#define hydro_sign_PREHASHBYTES 64

void hydro_sign_keygen(hydro_sign_keypair *kp)
{
    uint8_t *pk_copy = &kp->sk[X25519_LEN];
    hydro_random_buf(kp->sk, X25519_LEN);
    x25519_base(kp->pk, kp->sk);
    memcpy(pk_copy, kp->pk, X25519_LEN);
}

int hydro_sign_init(hydro_sign_state *state,
                    const char ctx[hydro_sign_CONTEXTBYTES])
{
    return hydro_hash_init(&state->hash_st, ctx, NULL);
}

int hydro_sign_update(hydro_sign_state *state, const void *m_, size_t mlen)
{
    return hydro_hash_update(&state->hash_st, m_, mlen);
}

static void hydro_sign_challenge(uint8_t challenge[hydro_sign_CHALLENGEBYTES],
                                 const uint8_t nonce[hydro_sign_NONCEBYTES],
                                 const uint8_t pk[hydro_sign_PUBLICKEYBYTES],
                                 const uint8_t prehash[hydro_sign_PREHASHBYTES])
{
    hydro_hash_state st;
    hydro_hash_init(&st, NULL, NULL);
    hydro_hash_update(&st, nonce, hydro_sign_NONCEBYTES);
    hydro_hash_update(&st, pk, hydro_sign_PUBLICKEYBYTES);
    hydro_hash_update(&st, prehash, hydro_sign_PREHASHBYTES);
    hydro_hash_final(&st, challenge, hydro_sign_CHALLENGEBYTES);
}

int hydro_sign_final_create(hydro_sign_state *state,
                            uint8_t csig[hydro_sign_BYTES],
                            const uint8_t sk[hydro_sign_SECRETKEYBYTES])
{
    uint8_t prehash[hydro_sign_PREHASHBYTES];
    hydro_hash_final(&state->hash_st, prehash, sizeof prehash);

    hydro_hash_state st;
    uint8_t challenge[hydro_sign_CHALLENGEBYTES];
    const uint8_t *pk = &sk[X25519_LEN];
    uint8_t *nonce = &csig[0];
    uint8_t *sig = &csig[hydro_sign_NONCEBYTES];
    uint8_t *eph_sk = sig;

    hydro_random_buf(eph_sk, X25519_LEN);
    hydro_hash_init(&st, NULL, sk);
    hydro_hash_update(&st, eph_sk, X25519_LEN);
    hydro_hash_update(&st, prehash, hydro_sign_PREHASHBYTES);
    hydro_hash_final(&st, eph_sk, X25519_LEN);

    x25519_base(nonce, eph_sk);
    hydro_sign_challenge(challenge, nonce, pk, prehash);

    x25519_sign(sig, challenge, eph_sk, sk);

    return 0;
}

int hydro_sign_final_verify(hydro_sign_state *state,
                            const uint8_t csig[hydro_sign_BYTES],
                            const uint8_t pk[hydro_sign_PUBLICKEYBYTES])
{
    uint8_t challenge[hydro_sign_CHALLENGEBYTES];
    uint8_t prehash[hydro_sign_PREHASHBYTES];
    const uint8_t *nonce = &csig[0];

    hydro_hash_final(&state->hash_st, prehash, sizeof prehash);
    hydro_sign_challenge(challenge, nonce, pk, prehash);

    const uint8_t *sig = &csig[hydro_sign_NONCEBYTES];

    return x25519_verify(sig, challenge, nonce, pk) ? 0 : -1;
}

int hydro_sign_create(uint8_t csig[hydro_sign_BYTES], const void *m_,
                      size_t mlen, const char ctx[hydro_sign_CONTEXTBYTES],
                      const uint8_t sk[hydro_sign_SECRETKEYBYTES])
{
    hydro_sign_state st;

    if (hydro_sign_init(&st, ctx) != 0 ||
        hydro_sign_update(&st, m_, mlen) != 0 ||
        hydro_sign_final_create(&st, csig, sk) != 0)
    {
        return -1;
    }
    return 0;
}

int hydro_sign_verify(const uint8_t csig[hydro_sign_BYTES], const void *m_,
                      size_t mlen, const char ctx[hydro_sign_CONTEXTBYTES],
                      const uint8_t pk[hydro_sign_PUBLICKEYBYTES])
{
    hydro_sign_state st;

    if (hydro_sign_init(&st, ctx) != 0 ||
        hydro_sign_update(&st, m_, mlen) != 0 ||
        hydro_sign_final_verify(&st, csig, pk) != 0)
    {
        return -1;
    }
    return 0;
}
