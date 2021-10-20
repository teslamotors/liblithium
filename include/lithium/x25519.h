#ifndef LITHIUM_X25519_H
#define LITHIUM_X25519_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's x25519.h, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>

#define X25519_BITS 256
#define X25519_LEN (X25519_BITS / 8)

/*
 * x25519 scalar multiplication. Sets out to scalar*point.
 *
 * scalar will be clamped internally per RFC7748.
 */
void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char point[X25519_LEN]);

/*
 * Scalar multiplication of the curve's base point.
 *
 * scalar will be clamped internally per RFC7748.
 */
void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN]);

/*
 * Reduce a 512-bit scalar s mod L, the prime subgroup order.
 * Store the result in r. s and r may alias.
 */
void x25519_scalar_reduce(unsigned char r[X25519_LEN],
                          const unsigned char s[X25519_LEN * 2]);

/*
 * Scalar multiplication of the curve's base point by a scalar which is
 * uniformly-distributed mod L, the prime subgroup order.
 */
void x25519_base_uniform(unsigned char out[X25519_LEN],
                         const unsigned char scalar[X25519_LEN]);

/*
 * Schnorr signatures using Curve25519 (not ed25519).
 *
 * The user will call x25519_base_uniform(public_nonce, secret_nonce) to
 * schedule a random ephemeral secret key. They then call a Schnorr oracle to
 * get a challenge, and compute the response using this function.
 *
 * challenge and secret_nonce should be uniform mod L.
 */
void x25519_sign(unsigned char response[X25519_LEN],
                 const unsigned char challenge[X25519_LEN],
                 const unsigned char secret_nonce[X25519_LEN],
                 const unsigned char secret_scalar[X25519_LEN]);

/*
 * Signature verification using Curve25519 (not ed25519).
 * This function is the public equivalent x25519_sign, taking the long-term
 * and ephemeral public keys instead of secret ones.
 *
 * Returns true for a matching signature.
 */
bool x25519_verify(const unsigned char response[X25519_LEN],
                   const unsigned char challenge[X25519_LEN],
                   const unsigned char public_nonce[X25519_LEN],
                   const unsigned char public_key[X25519_LEN]);

#endif /* LITHIUM_X25519_H */
