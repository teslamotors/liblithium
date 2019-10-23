#include <lithium/sign.h>

#include <lithium/random.h>
#include <lithium/x25519.h>

#include <string.h>

#define PREHASH_LEN 64
#define AZ_LEN 64

static void gen_az(unsigned char az[AZ_LEN],
                   const unsigned char secret_key[X25519_LEN])
{
    gimli_hash(az, AZ_LEN, secret_key, X25519_LEN);
    x25519_clamp(az);
}

void lith_sign_keygen(unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
                      unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN])
{
    lith_random_bytes(secret_key, X25519_LEN);
    unsigned char az[AZ_LEN];
    gen_az(az, secret_key);
    x25519_base(public_key, az);
    memcpy(&secret_key[X25519_LEN], public_key, X25519_LEN);
}

void lith_sign_init(lith_sign_state *state)
{
    gimli_hash_init(state);
}

void lith_sign_update(lith_sign_state *state, const unsigned char *msg,
                      size_t len)
{
    gimli_hash_update(state, msg, len);
}

static void
gen_challenge(gimli_hash_state *state, unsigned char challenge[X25519_LEN],
              const unsigned char R[X25519_LEN],
              const unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
              const unsigned char prehash[PREHASH_LEN])
{
    gimli_hash_init(state);
    gimli_hash_update(state, R, X25519_LEN);
    gimli_hash_update(state, public_key, LITH_SIGN_PUBLIC_KEY_LEN);
    gimli_hash_update(state, prehash, PREHASH_LEN);
    gimli_hash_final(state, challenge, X25519_LEN);
    x25519_clamp(challenge);
}

void lith_sign_final_create(
    lith_sign_state *state, unsigned char sig[LITH_SIGN_LEN],
    const unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN])
{
    unsigned char prehash[PREHASH_LEN];
    gimli_hash_final(state, prehash, PREHASH_LEN);

    unsigned char *const public_nonce = &sig[0];
    unsigned char *const response = &sig[X25519_LEN];
    const unsigned char *const public_key = &secret_key[X25519_LEN];

    unsigned char az[AZ_LEN];
    gen_az(az, secret_key);

    // use response part of signature as scratch space for the secret nonce
    unsigned char *const secret_nonce = response;
    gimli_hash_init(state);
    gimli_hash_update(state, &az[X25519_LEN], X25519_LEN);
    gimli_hash_update(state, prehash, PREHASH_LEN);
    gimli_hash_final(state, secret_nonce, X25519_LEN);
    x25519_clamp(secret_nonce);
    x25519_base(public_nonce, secret_nonce);

    unsigned char challenge[X25519_LEN];
    gen_challenge(state, challenge, public_nonce, public_key, prehash);

    x25519_sign(response, challenge, secret_nonce, az);
}

bool lith_sign_final_verify(
    lith_sign_state *state, const unsigned char sig[LITH_SIGN_LEN],
    const unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN])
{
    unsigned char prehash[PREHASH_LEN];
    gimli_hash_final(state, prehash, PREHASH_LEN);

    const unsigned char *const public_nonce = &sig[0];
    const unsigned char *const response = &sig[X25519_LEN];

    // reuse prehash buffer for challenge
    unsigned char *const challenge = prehash;
    gen_challenge(state, challenge, public_nonce, public_key, prehash);

    return x25519_verify(response, challenge, public_nonce, public_key);
}

void lith_sign_create(unsigned char sig[LITH_SIGN_LEN], const void *msg,
                      size_t len,
                      const unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN])
{
    lith_sign_state state;
    lith_sign_init(&state);
    lith_sign_update(&state, msg, len);
    lith_sign_final_create(&state, sig, secret_key);
}

bool lith_sign_verify(const unsigned char sig[LITH_SIGN_LEN], const void *msg,
                      size_t len,
                      const unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN])
{
    lith_sign_state state;
    lith_sign_init(&state);
    lith_sign_update(&state, msg, len);
    return lith_sign_final_verify(&state, sig, public_key);
}
