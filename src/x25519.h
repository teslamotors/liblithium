/**
 * @file x25519.h
 * @copyright
 *   Copyright (c) 2016 Cryptography Research, Inc.
 *   Released under the MIT License. See LICENSE for license information.
 * @author Mike Hamburg
 * @brief X25519 key exchange and signatures.
 */

#pragma once

#define X25519_BYTES (256 / 8)

/* The base point (9) */
extern const unsigned char X25519_BASE_POINT[X25519_BYTES];

/** Number of bytes in an EC public key */
#define EC_PUBLIC_BYTES 32

/** Number of bytes in an EC private key */
#define EC_PRIVATE_BYTES 32

/**
 * Number of bytes in a Schnorr challenge.
 * Could be set to 16 in a pinch.  (FUTURE?)
 */
#define EC_CHALLENGE_BYTES 32

/** Enough bytes to get a uniform sample mod #E.  For eg a Brainpool
 * curve this would need to be more than for a private key, but due
 * to the special prime used by Curve25519, the same size is enough.
 */
#define EC_UNIFORM_BYTES 32

/* x25519 scalar multiplication.  Sets out to scalar*base.
 *
 * If clamp is set then the scalar will be "clamped" like a Curve25519 secret
 * key.  This adds almost no security, but permits interop with other x25519
 * implementations without manually clamping the keys.
 *
 * Per RFC 7748, this function returns failure (-1) if the output
 * is zero and clamp is set.  This indicates "non-contributory behavior",
 * meaning that one party might steer the key so that the other party's
 * contribution doesn't matter, or contributes only a little entropy.
 *
 * If clamp==0, then this function always returns 0.
 */
int x25519(unsigned char out[EC_PUBLIC_BYTES],
           const unsigned char scalar[EC_PRIVATE_BYTES],
           const unsigned char base[EC_PUBLIC_BYTES], int clamp);

/**
 * Returns 0 on success, -1 on failure.
 *
 * Per RFC 7748, this function returns failure if the output
 * is zero and clamp is set.  This usually doesn't matter for
 * base scalarmuls.
 *
 * If clamp==0, then this function always returns 0.
 *
 * Same as x255(out,scalar,X255_BASE_POINT), except that
 * other implementations may optimize it.
 */
static inline int x25519_base(unsigned char out[EC_PUBLIC_BYTES],
                              const unsigned char scalar[EC_PRIVATE_BYTES],
                              int clamp)
{
    return x25519(out, scalar, X25519_BASE_POINT, clamp);
}

/**
 * As x25519_base, but with a scalar that's EC_UNIFORM_BYTES long,
 * and clamp always 0 (and thus, no return value).
 *
 * This is used for signing.  Implementors must replace it for
 * curves that require more bytes for uniformity (Brainpool).
 */
static inline void
x25519_base_uniform(unsigned char out[EC_PUBLIC_BYTES],
                    const unsigned char scalar[EC_UNIFORM_BYTES])
{
    (void)x25519_base(out, scalar, 0);
}

/**
 * STROBE-compatible Schnorr signatures using curve25519 (not ed25519)
 *
 * The user will call x25519_base_uniform(eph,eph_secret) to schedule
 * a random ephemeral secret key.  They then call a Schnorr oracle to
 * get a challenge, and compute the response using this function.
 */
void x25519_sign_p2(unsigned char response[EC_PRIVATE_BYTES],
                    const unsigned char challenge[EC_CHALLENGE_BYTES],
                    const unsigned char eph_secret[EC_UNIFORM_BYTES],
                    const unsigned char secret[EC_PRIVATE_BYTES]);

/**
 * STROBE-compatible signature verification using curve25519 (not ed25519).
 * This function is the public equivalent x25519_sign_p2, taking the long-term
 * and ephemeral public keys instead of secret ones.
 *
 * Returns -1 on failure and 0 on success.
 */
int x25519_verify_p2(const unsigned char response[X25519_BYTES],
                     const unsigned char challenge[X25519_BYTES],
                     const unsigned char eph[X25519_BYTES],
                     const unsigned char pub[X25519_BYTES]);
