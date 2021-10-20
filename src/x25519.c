/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's x25519.c, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <lithium/x25519.h>

#include <lithium/watchdog.h>

#include "fe.h"

#include <stdint.h>
#include <string.h>

typedef limb sc[NLIMBS];
typedef limb feq[NLIMBS * 2];

static limb *X(feq P)
{
    return P;
}

static limb *Z(feq P)
{
    return &P[NLIMBS];
}

static void cswap(limb swap, feq P, feq Q)
{
    int i;
    for (i = 0; i < NLIMBS * 2; ++i)
    {
        const limb d = (P[i] ^ Q[i]) & swap;
        P[i] ^= d;
        Q[i] ^= d;
    }
}

/*
 * Curve constant a = 486662
 * (a - 2)/4 = 121665 = 0x1DB41
 */

static void mul_a24(fe out, const fe a)
{
#if (LITH_X25519_WBITS < 32)
    static const fe a24 = {LIMBS(0xDB41, 0x0001, 0x0000, 0x0000)};
    mul(out, a, a24);
#else
    mul_word(out, a, 121665);
#endif
}

static void ladder_part1(feq P, feq Q, fe t)
{
    add(t, X(P), Z(P));    /* t = A = x + z */
    sub(Z(P), X(P), Z(P)); /* Z(P) = B = x - z */
    add(X(P), X(Q), Z(Q)); /* X(P) = C = u + w */
    sub(Z(Q), X(Q), Z(Q)); /* Z(Q) = D = u - w */
    mul1(Z(Q), t);         /* Z(Q) = DA = (u - w)(x + z) = xu + zu - xw - zw */
    mul1(X(P), Z(P));      /* X(Q) = CB = (u + w)(x - z) = xu - zu + xw - zw */
    add(X(Q), Z(Q), X(P)); /* X(Q) = DA + CB = 2xu - 2zw */
    sub(Z(Q), Z(Q), X(P)); /* Z(Q) = DA - CB = 2zu - 2xw */
    sqr1(t);               /* t = AA = (x + z)^2 = xx + 2xz + zz */
    sqr1(Z(P));            /* Z(P) = BB = (x - z)^2 = xx - 2xz + zz */
    sub(X(P), t, Z(P));    /* X(P) = E = AA - BB = 4xz */
    mul_a24(Z(P), X(P));   /* Z(P) = E(a - 2)/4 = 4xz(a - 2)/4 = axz - 2xz */
    add(Z(P), Z(P), t);    /* Z(P) = E(a - 2)/4 + AA = xx + axz + zz */
}

static void ladder_part2(feq P, feq Q, const fe t, const fe x)
{
    sqr1(Z(Q));         /* Z(Q) = (DA - CB)^2 */
    mul1(Z(Q), x);      /* Z(Q) = x(DA - CB)^2 */
    sqr1(X(Q));         /* X(Q) = (DA + CB)^2 */
    mul1(Z(P), X(P));   /* Z(P) = E(E(a - 2)/4 + AA) */
    sub(X(P), t, X(P)); /* X(P) = AA - E = AA - (AA - BB) = BB */
    mul1(X(P), t);      /* X(P) = AABB */
}

static void x25519_q(feq P, const sc k, const fe x)
{
    feq Q;
    limb swap = 0;
    int i;
    (void)memcpy(X(Q), x, sizeof(fe));
    (void)memset(Z(Q), 0, sizeof(fe));
    (void)memset(P, 0, sizeof(feq));
    Z(Q)[0] = 1;
    X(P)[0] = 1;

    for (i = X25519_BITS - 1; i >= 0; --i)
    {
        fe t;
        const limb ki =
            ~((k[i / LITH_X25519_WBITS] >> (i % LITH_X25519_WBITS)) & 1) + 1;
        cswap(swap ^ ki, P, Q);
        swap = ki;
        ladder_part1(P, Q, t);
        ladder_part2(P, Q, t, x);
#if (LITH_ENABLE_WATCHDOG)
        lith_watchdog_pet();
#endif
    }
    cswap(swap, P, Q);
}

static void feq_to_bytes(unsigned char out[X25519_LEN], feq P)
{
    inv(Z(P));
    mul1(X(P), Z(P));
    (void)canon(X(P));
    write_limbs(out, X(P));
}

void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char point[X25519_LEN])
{
    feq P;
    fe x;
    sc k;

    /*
     * Per RFC7748 section 5:
     * "When receiving such an array, implementations of X25519 (but not X448)
     * MUST mask the most significant bit in the final byte. This is done to
     * preserve compatibility with point formats that reserve the sign bit for
     * use in other protocols and to increase resistance to implementation
     * fingerprinting."
     *
     * Clear this bit after converting to an fe to avoid making an extra copy.
     */
    read_limbs(x, point);
    x[NLIMBS - 1] &= ~LIMB_HIGH_BIT_MASK;

    /*
     * Also per RFC7748 section 5:
     * "For X25519, in order to decode 32 random bytes as an integer scalar,
     * set the three least significant bits of the first byte and the most
     * significant bit of the last to zero, set the second most significant bit
     * of the last byte to 1 and, finally, decode as little-endian."
     *
     * Perform these bit set/clear operations after converting to an sc to
     * avoid making an extra copy.
     */
    read_limbs(k, scalar);
    k[0] &= ~(limb)0x7U;
    k[NLIMBS - 1] &= ~LIMB_HIGH_BIT_MASK;
    k[NLIMBS - 1] |= LIMB_HIGH_BIT_MASK >> 1;

    x25519_q(P, k, x);
    feq_to_bytes(out, P);
}

#define BASE_POINT 9U

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    const unsigned char base_point[X25519_LEN] = {BASE_POINT};
    x25519(out, scalar, base_point);
}

static const sc L = {
    LIMBS(0xD3ED, 0x5CF5, 0x631A, 0x5812),
    LIMBS(0x9CD6, 0xA2F7, 0xF9DE, 0x14DE),
    LIMBS(0x0000, 0x0000, 0x0000, 0x0000),
    LIMBS(0x0000, 0x0000, 0x0000, 0x1000),
};

static const sc RmodL = {
    LIMBS(0x951D, 0x8D98, 0x3174, 0xD6EC),
    LIMBS(0xCF70, 0x737D, 0x5BF4, 0xC6EF),
    LIMBS(0xFFFE, 0xFFFF, 0xFFFF, 0xFFFF),
    LIMBS(0xFFFF, 0xFFFF, 0xFFFF, 0x0FFF),
};

static const sc R2modL = {
    LIMBS(0x0F01, 0x449C, 0x11E3, 0xA406),
    LIMBS(0x9347, 0x6885, 0x1BA7, 0xD00E),
    LIMBS(0xBE65, 0x17F5, 0x73D2, 0xCEEC),
    LIMBS(0x9A3D, 0x7C30, 0x411B, 0x0399),
};

/*
 * Montgomery factor: -L^-1 mod 2^LITH_X25519_WBITS
 */
#define MONTGOMERY_FACTOR LOW_LIMB(0x7E1B, 0x1254, 0x1DA3, 0xD2B5)

/*
 * Set t = (t + ab)R^-1 mod L
 */
static void sc_montmul(sc t, const sc a, const sc b)
{
    /*
     * OK, so carry bounding. We're using a high carry, so that the inputs
     * don't have to be reduced.
     *
     * First montmul: output < (M^2 + Mp)/M = M+p, subtract p, < M. This gets
     * rid of high carry. Second montmul, by r^2 mod p < p: output < (Mp +
     * Mp)/M = 2p, subtract p, < p, done.
     */
    limb hic = 0, need_add, carry, carry2, u;
    sdlimb scarry = 0;
    int i, j;

    for (i = 0; i < NLIMBS; ++i)
    {
        carry = 0;
        carry2 = 0;
        u = MONTGOMERY_FACTOR;
        for (j = 0; j < NLIMBS; ++j)
        {
            limb acc = t[j];
            acc = mac(&carry, acc, a[i], b[j]);
            if (j == 0)
            {
                u *= acc;
            }
            acc = mac(&carry2, acc, u, L[j]);
            if (j > 0)
            {
                t[j - 1] = acc;
            }
        }

        /* Add two carry registers and high carry */
        t[NLIMBS - 1] = adc(&hic, carry, carry2);
    }

    /* Reduce */
    for (i = 0; i < NLIMBS; ++i)
    {
        scarry = scarry + t[i] - L[i];
        t[i] = (limb)scarry;
        scarry = asr(scarry, LITH_X25519_WBITS);
    }
    need_add = (limb)(-(scarry + hic));

    carry = 0;
    for (i = 0; i < NLIMBS; ++i)
    {
        t[i] = mac(&carry, t[i], need_add, L[i]);
    }
}

void x25519_scalar_reduce(unsigned char r[X25519_LEN],
                          const unsigned char s[X25519_LEN * 2])
{
    /*
     * R = 2^256
     * let a = s mod R (lower 256 bits)
     * let b = s / R (upper 256 bits)
     * s mod L = (a + bR) mod L
     *
     * sc_montmul(t, a, b) implements:
     * t = (t + ab)R^-1 mod L
     *
     * sc_montmul(a, b, RmodL):
     * a' = (a + bR)R^-1 mod L
     *
     * b' = 0
     * sc_montmul(b', a', R^2):
     * b'' = (b' + a'R^2)R^-1 mod L
     * b'' = (0 + ((a + bR)R^-1)R^2)R^-1 mod L
     * b'' = (a + bR) mod L
     */
    sc a, b;
    read_limbs(a, &s[0]);
    read_limbs(b, &s[X25519_LEN]);
    sc_montmul(a, b, RmodL);
    (void)memset(b, 0, sizeof b);
    sc_montmul(b, a, R2modL);
    write_limbs(r, b);
}

void x25519_base_uniform(unsigned char out[X25519_LEN],
                         const unsigned char scalar[X25519_LEN])
{
    feq P;
    fe B = {BASE_POINT};
    sc k;
    read_limbs(k, scalar);
    x25519_q(P, k, B);
    feq_to_bytes(out, P);
}

bool x25519_verify(const unsigned char response[X25519_LEN],
                   const unsigned char challenge[X25519_LEN],
                   const unsigned char public_nonce[X25519_LEN],
                   const unsigned char public_key[X25519_LEN])
{
    /*
     * See doc/verify.tex for a derivation of signature verification using only
     * x-coordinates based on "Fast and compact elliptic-curve cryptography".
     * https://www.shiftleft.org/papers/fff/
     * https://eprint.iacr.org/2012/309.pdf
     */
    feq P, Q;
    fe A, B = {BASE_POINT};
    sc k;

    read_limbs(k, response);
    x25519_q(P, k, B);
    /* P = x/z = response*base_point */

    read_limbs(k, challenge);
    read_limbs(A, public_key);
    x25519_q(Q, k, A);
    /* Q = u/w = challenge*public_key */

    mul(A, X(Q), Z(Q));
    mul_word(A, A, 16);
    /* A = 16uw */

    ladder_part1(P, Q, B);
    /* X(Q) = 2xu - 2zw */
    /* Z(Q) = 2zu - 2xw */
    /* Z(P) = xx + axz + zz */

    read_limbs(B, public_nonce);
    /* B = R */

    mul1(A, Z(P));
    mul1(A, B);
    /* A = left = 16uwR(xx + axz + zz) */

    mul1(Z(Q), B);
    sub(B, Z(Q), X(Q));
    sqr1(B);
    /* B = right = (R(2zu - 2xw) - (2xu - 2zw))^2 */

    /* check equality:
     * 16uwR(xx + axz + zz) == (R(2zu - 2xw) - (2xu - 2zw))^2 */
    sub(A, A, B);

    /*
     * If canon(A) returns nonzero, then A is zero and the two sides are equal.
     * If canon(B) also returns nonzero, then B is zero and both sides are zero.
     *
     * Reject signatures where both sides are zero, because that can happen if
     * an input causes the ladder to return 0/0.
     */
    return (canon(A) & ~canon(B)) != 0;
}

/*
 * compute response = secret_nonce + secret_scalar * challenge mod L
 *
 * verifier will check
 * response * base = (secret_nonce + secret_scalar * challenge) * base
 * response * base = secret_nonce * base + secret_scalar * challenge * base
 * response * base = public_nonce + challenge * public_key
 */
void x25519_sign(unsigned char response[X25519_LEN],
                 const unsigned char challenge[X25519_LEN],
                 const unsigned char secret_nonce[X25519_LEN],
                 const unsigned char secret_scalar[X25519_LEN])
{
    /*
     * R = 2^256
     * let r = secret_nonce
     * let a = secret_scalar
     * let h = challenge
     *
     * sc_montmul(t, a, b) implements:
     * t = (t + ab)R^-1 mod L
     *
     * sc_montmul(r, a, h):
     * r' = (r + ah)R^-1 mod L
     *
     * a' = 0
     * sc_montmul(a', r', R^2):
     * a'' = (a' + r'R^2)R^-1 mod L
     * a'' = (0 + ((r + ah)R^-1)R^2)R^-1 mod L
     * a'' = (r + ah) mod L
     * a'' = secret_nonce + secret_scalar * challenge mod L
     */
    sc r, a, h;
    read_limbs(r, secret_nonce);
    read_limbs(a, secret_scalar);
    read_limbs(h, challenge);
    sc_montmul(r, a, h);
    (void)memset(a, 0, sizeof a);
    sc_montmul(a, r, R2modL);
    write_limbs(response, a);
}
