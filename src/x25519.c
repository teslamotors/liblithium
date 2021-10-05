/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's x25519.c, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <lithium/x25519.h>

#include <lithium/fe.h>
#include <lithium/watchdog.h>

#include "carry.h"

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

static void ladder_part1(feq P, feq Q, fe t)
{
    const fe a24 = {LIMBS(0xDB41, 0x0001, 0x0000, 0x0000)};
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
    mul(Z(P), X(P), a24);  /* Z(P) = E(a - 2)/4 = 4xz(a - 2)/4 = axz - 2xz */
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

static void x25519_q(feq P, const unsigned char k[X25519_LEN], const fe x)
{
    feq Q = {0};
    limb swap = 0;
    int i;
    Z(Q)[0] = 1;
    (void)memcpy(X(Q), x, sizeof(fe));
    (void)memset(P, 0, sizeof(feq));
    X(P)[0] = 1;

    for (i = X25519_BITS - 1; i >= 0; --i)
    {
        const limb ki = ~(((limb)k[i / 8] >> (i % 8)) & 1) + 1;
        fe t;
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

void x25519_clamp(unsigned char scalar[X25519_LEN])
{
    scalar[0] &= 0xF8U;
    scalar[X25519_LEN - 1] &= 0x7FU;
    scalar[X25519_LEN - 1] |= 0x40U;
}

void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char point[X25519_LEN])
{
    fe x;
    feq P;
    read_limbs(x, point);
    x25519_q(P, scalar, x);
    inv(Z(P), Z(P));
    mul1(X(P), Z(P));
    (void)canon(X(P));
    write_limbs(out, X(P));
}

#define BASE_POINT 9U

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    const unsigned char base_point[X25519_LEN] = {BASE_POINT};
    x25519(out, scalar, base_point);
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
    read_limbs(A, public_key);
    x25519_q(P, response, B);
    x25519_q(Q, challenge, A);
    /* P = x/z = response*base_point */
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

    mul1(Z(P), A);
    mul1(Z(P), B);
    /* Z(P) = left = 16uwR(xx + axz + zz) */

    mul1(Z(Q), B);
    sub(Z(Q), Z(Q), X(Q));
    sqr1(Z(Q));
    /* Z(Q) = right = (R(2zu - 2xw) - (2xu - 2zw))^2 */

    /* check equality */
    sub(Z(Q), Z(Q), Z(P));

    /*
     * If canon(Z(Q)) then the two sides are equal.
     * If canon(Z(P)) also, then both sides are zero.
     *
     * Reject signatures where both sides are zero, because that can happen if
     * an input causes the ladder to return 0/0.
     */
    return (bool)(canon(Z(Q)) & ~canon(Z(P)));
}

/*
 * Montgomery factor: -L^-1 mod 2^LITH_X25519_WBITS
 */
#define MONTGOMERY_FACTOR LOW_LIMB(0x7E1B, 0x1254, 0x1DA3, 0xD2B5)

/*
 * Set t = (t + ab)R^-1 mod l
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
    static const sc L = {
        LIMBS(0xD3ED, 0x5CF5, 0x631A, 0x5812),
        LIMBS(0x9CD6, 0xA2F7, 0xF9DE, 0x14DE),
        LIMBS(0x0000, 0x0000, 0x0000, 0x0000),
        LIMBS(0x0000, 0x0000, 0x0000, 0x1000),
    };

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

/*
 * compute response = secret_nonce + secret_key * challenge
 *
 * verifier will check
 * response * base = (secret_nonce + secret_key * challenge) * base
 * response * base = secret_nonce * base + secret_key * challenge * base
 * response * base = public_nonce + challenge * public_key
 */
void x25519_sign(unsigned char response[X25519_LEN],
                 const unsigned char challenge[X25519_LEN],
                 const unsigned char secret_nonce[X25519_LEN],
                 const unsigned char secret_key[X25519_LEN])
{
    /*
     * R = 2^256
     * Doing a Montgomery multiply by R^2 mod L undoes the Montgomery
     * reductions.
     */
    static const sc R2modL = {
        LIMBS(0x0F01, 0x449C, 0x11E3, 0xA406),
        LIMBS(0x9347, 0x6885, 0x1BA7, 0xD00E),
        LIMBS(0xBE65, 0x17F5, 0x73D2, 0xCEEC),
        LIMBS(0x9A3D, 0x7C30, 0x411B, 0x0399),
    };
    sc r, a, h;
    read_limbs(r, secret_nonce);
    read_limbs(a, secret_key);
    read_limbs(h, challenge);
    sc_montmul(r, a, h); /* r = (secret_nonce + secret_key * challenge)R^-1 */
    (void)memset(a, 0, sizeof(sc));
    sc_montmul(a, r, R2modL); /* a = (secret_nonce + secret_key * challenge) */
    write_limbs(response, a);
}
