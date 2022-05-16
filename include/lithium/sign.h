#ifndef LITHIUM_SIGN_H
#define LITHIUM_SIGN_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli_hash.h>

#include <stdbool.h>

/* cffi:begin */

#define LITH_SIGN_LEN 64
#define LITH_SIGN_PUBLIC_KEY_LEN 32
#define LITH_SIGN_SECRET_KEY_LEN 64
#define LITH_SIGN_PREHASH_LEN 64

typedef gimli_hash_state lith_sign_state;

void lith_sign_keygen(unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
                      unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN]);

void lith_sign_init(lith_sign_state *state);

void lith_sign_update(lith_sign_state *state, const unsigned char *msg,
                      size_t len);

void lith_sign_final_create(lith_sign_state *state,
                            unsigned char sig[LITH_SIGN_LEN],
                            const unsigned char
                                secret_key[LITH_SIGN_SECRET_KEY_LEN]);

bool lith_sign_final_verify(lith_sign_state *state,
                            const unsigned char sig[LITH_SIGN_LEN],
                            const unsigned char
                                public_key[LITH_SIGN_PUBLIC_KEY_LEN]);

void lith_sign_final_prehash(lith_sign_state *state,
                             unsigned char prehash[LITH_SIGN_PREHASH_LEN]);

void lith_sign_create_from_prehash(unsigned char sig[LITH_SIGN_LEN],
                                   const unsigned char
                                       prehash[LITH_SIGN_PREHASH_LEN],
                                   const unsigned char
                                       secret_key[LITH_SIGN_SECRET_KEY_LEN]);

bool lith_sign_verify_prehash(const unsigned char sig[LITH_SIGN_LEN],
                              const unsigned char
                                  prehash[LITH_SIGN_PREHASH_LEN],
                              const unsigned char
                                  public_key[LITH_SIGN_PUBLIC_KEY_LEN]);

void lith_sign_create(unsigned char sig[LITH_SIGN_LEN],
                      const unsigned char *msg, size_t len,
                      const unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN]);

bool lith_sign_verify(const unsigned char sig[LITH_SIGN_LEN],
                      const unsigned char *msg, size_t len,
                      const unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN]);

/* cffi:end */

#endif /* LITHIUM_SIGN_H */
