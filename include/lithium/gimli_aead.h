#pragma once

#include <stdbool.h>
#include <stddef.h>

#define GIMLI_AEAD_NONCE_LEN 16
#define GIMLI_AEAD_KEY_LEN 32
#define GIMLI_AEAD_TAG_DEFAULT_LEN 16

void gimli_aead_encrypt(unsigned char *c, unsigned char *t, size_t tlen,
                        const unsigned char *m, size_t mlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN]);

bool gimli_aead_decrypt(unsigned char *m, const unsigned char *c, size_t clen,
                        const unsigned char *t, size_t tlen,
                        const unsigned char *ad, size_t adlen,
                        const unsigned char n[GIMLI_AEAD_NONCE_LEN],
                        const unsigned char k[GIMLI_AEAD_KEY_LEN])
    __attribute__((warn_unused_result));
