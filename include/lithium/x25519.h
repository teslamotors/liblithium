/*
 * Copyright (c) 2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#pragma once

#define X25519_BITS 256
#define X25519_LEN (X25519_BITS / 8)

/*
 * x25519 scalar multiplication. Sets out to scalar*base.
 *
 * Scalar should be clamped like a Curve25519 secret key.
 */
void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char base[X25519_LEN]);

/*
 * Scalar multiplication of the curve's base point.
 */
void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN]);

/*
 * STROBE-compatible Schnorr signatures using curve25519 (not ed25519).
 *
 * The user will call x25519_base(eph, eph_secret) to schedule a random
 * ephemeral secret key. They then call a Schnorr oracle to get a challenge,
 * and compute the response using this function.
 */
void x25519_sign_p2(unsigned char response[X25519_LEN],
                    const unsigned char challenge[X25519_LEN],
                    const unsigned char eph_secret[X25519_LEN],
                    const unsigned char secret[X25519_LEN]);

/*
 * STROBE-compatible signature verification using curve25519 (not ed25519).
 * This function is the public equivalent x25519_sign_p2, taking the long-term
 * and ephemeral public keys instead of secret ones.
 *
 * Returns -1 on failure and 0 on success.
 */
int x25519_verify_p2(const unsigned char response[X25519_LEN],
                     const unsigned char challenge[X25519_LEN],
                     const unsigned char eph[X25519_LEN],
                     const unsigned char pub[X25519_LEN]);
