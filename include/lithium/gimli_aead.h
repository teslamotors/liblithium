#ifndef LITHIUM_GIMLI_AEAD_H
#define LITHIUM_GIMLI_AEAD_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli_state.h>

#include <stdbool.h>
#include <stddef.h>

#define GIMLI_AEAD_NONCE_LEN 16
#define GIMLI_AEAD_KEY_LEN 32
#define GIMLI_AEAD_TAG_DEFAULT_LEN 16

void gimli_aead_init(gimli_state *g,
                     const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                     const unsigned char k[GIMLI_AEAD_KEY_LEN]);

void gimli_aead_update_ad(gimli_state *g, const unsigned char *ad,
                          size_t adlen);

void gimli_aead_final_ad(gimli_state *g);

void gimli_aead_encrypt_update(gimli_state *g, unsigned char *c,
                               const unsigned char *m, size_t len);

void gimli_aead_encrypt_final(gimli_state *g, unsigned char *t, size_t tlen);

void gimli_aead_decrypt_update(gimli_state *g, unsigned char *m,
                               const unsigned char *c, size_t len);

bool gimli_aead_decrypt_final(gimli_state *g, const unsigned char *t,
                              size_t tlen);

void gimli_aead_encrypt(unsigned char *c, unsigned char *t, size_t tlen,
                        const unsigned char *m, size_t len,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN]);

bool gimli_aead_decrypt(unsigned char *m, const unsigned char *c, size_t len,
                        const unsigned char *t, size_t tlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN]);

#endif /* LITHIUM_GIMLI_AEAD_H */
