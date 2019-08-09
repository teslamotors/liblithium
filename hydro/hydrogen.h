#ifndef hydrogen_H
#define hydrogen_H

#include <lithium/gimli_hash.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__) || defined(__GNUC__)
# define _hydro_attr_(X) __attribute__(X)
#else
# define _hydro_attr_(X)
#endif
#define _hydro_attr_deprecated_         _hydro_attr_((deprecated))
#define _hydro_attr_malloc_             _hydro_attr_((malloc))
#define _hydro_attr_noinline_           _hydro_attr_((noinline))
#define _hydro_attr_noreturn_           _hydro_attr_((noreturn))
#define _hydro_attr_warn_unused_result_ _hydro_attr_((warn_unused_result))
#define _hydro_attr_weak_               _hydro_attr_((weak))

#define HYDRO_VERSION_MAJOR 1
#define HYDRO_VERSION_MINOR 0

/* ---------------- */

void hydro_random_buf(void *out, size_t out_len);

/* ---------------- */

#define hydro_hash_BYTES 32
#define hydro_hash_BYTES_MAX 65535
#define hydro_hash_BYTES_MIN 16
#define hydro_hash_CONTEXTBYTES 8
#define hydro_hash_KEYBYTES 32

typedef gimli_hash_state hydro_hash_state;

void hydro_hash_keygen(uint8_t key[hydro_hash_KEYBYTES]);

int hydro_hash_init(hydro_hash_state *state, const char ctx[hydro_hash_CONTEXTBYTES],
                    const uint8_t key[hydro_hash_KEYBYTES]);

int hydro_hash_update(hydro_hash_state *state, const void *in_, size_t in_len);

int hydro_hash_final(hydro_hash_state *state, uint8_t *out, size_t out_len);

int hydro_hash_hash(uint8_t *out, size_t out_len, const void *in_, size_t in_len,
                    const char    ctx[hydro_hash_CONTEXTBYTES],
                    const uint8_t key[hydro_hash_KEYBYTES]);

/* ---------------- */

#define hydro_sign_BYTES 64
#define hydro_sign_CONTEXTBYTES 8
#define hydro_sign_PUBLICKEYBYTES 32
#define hydro_sign_SECRETKEYBYTES 64
#define hydro_sign_SEEDBYTES 32

typedef struct hydro_sign_state {
    hydro_hash_state hash_st;
} hydro_sign_state;

typedef struct hydro_sign_keypair {
    uint8_t pk[hydro_sign_PUBLICKEYBYTES];
    uint8_t sk[hydro_sign_SECRETKEYBYTES];
} hydro_sign_keypair;

void hydro_sign_keygen(hydro_sign_keypair *kp);

int hydro_sign_init(hydro_sign_state *state, const char ctx[hydro_sign_CONTEXTBYTES]);

int hydro_sign_update(hydro_sign_state *state, const void *m_, size_t mlen);

int hydro_sign_final_create(hydro_sign_state *state, uint8_t csig[hydro_sign_BYTES],
                            const uint8_t sk[hydro_sign_SECRETKEYBYTES]);

int hydro_sign_final_verify(hydro_sign_state *state, const uint8_t csig[hydro_sign_BYTES],
                            const uint8_t pk[hydro_sign_PUBLICKEYBYTES])
    _hydro_attr_warn_unused_result_;

int hydro_sign_create(uint8_t csig[hydro_sign_BYTES], const void *m_, size_t mlen,
                      const char    ctx[hydro_sign_CONTEXTBYTES],
                      const uint8_t sk[hydro_sign_SECRETKEYBYTES]);

int hydro_sign_verify(const uint8_t csig[hydro_sign_BYTES], const void *m_, size_t mlen,
                      const char    ctx[hydro_sign_CONTEXTBYTES],
                      const uint8_t pk[hydro_sign_PUBLICKEYBYTES]) _hydro_attr_warn_unused_result_;

#ifdef __cplusplus
}
#endif

#endif
