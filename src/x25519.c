/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#include <lithium/x25519.h>

#include <lithium/fe.h>

#include "carry.h"

#include <stdint.h>
#include <string.h>

#define WLEN (WBITS / 8)

static uint32_t read_limb(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void write_limb(unsigned char *p, uint32_t x)
{
    p[0] = (unsigned char)x & 0xFFU;
    p[1] = (unsigned char)(x >> 8) & 0xFFU;
    p[2] = (unsigned char)(x >> 16) & 0xFFU;
    p[3] = (unsigned char)(x >> 24) & 0xFFU;
}

static void read_limbs(uint32_t x[NLIMBS], const unsigned char *in)
{
    for (int i = 0; i < NLIMBS; ++i)
    {
        x[i] = read_limb(in + i * WLEN);
    }
}

static void write_limbs(unsigned char *out, const uint32_t x[NLIMBS])
{
    for (int i = 0; i < NLIMBS; ++i)
    {
        write_limb(out + i * WLEN, x[i]);
    }
}

typedef uint32_t scalar_t[NLIMBS];

struct xz
{
    fe_t x, z;
};

static void cswap(uint32_t swap, struct xz *a, struct xz *b)
{
    uint32_t *ap = &a->x[0], *bp = &b->x[0];
    for (int i = 0; i < NLIMBS * 2; ++i)
    {
        const uint32_t d = (ap[i] ^ bp[i]) & swap;
        ap[i] ^= d;
        bp[i] ^= d;
    }
}

// Curve constant a = 486662
// (a - 2)/4
#define A24 UINT32_C(121665)

static void ladder_part1(struct xz *P, struct xz *Q, fe_t t)
{
    add(t, P->x, P->z);        // t = A = x + z
    sub(P->z, P->x, P->z);     // P->z = B = x - z
    add(P->x, Q->x, Q->z);     // P->x = C = u + w
    sub(Q->z, Q->x, Q->z);     // Q->z = D = u - w
    mul1(Q->z, t);             // Q->z = DA = (u - w)(x + z) = xu + zu - xw - zw
    mul1(P->x, P->z);          // Q->x = CB = (u + w)(x - z) = xu - zu + xw - zw
    add(Q->x, Q->z, P->x);     // Q->x = DA + CB = 2xu - 2zw
    sub(Q->z, Q->z, P->x);     // Q->z = DA - CB = 2zu - 2xw
    sqr1(t);                   // t = AA = (x + z)^2 = xx + 2xz + zz
    sqr1(P->z);                // P->z = BB = (x - z)^2 = xx - 2xz + zz
    sub(P->x, t, P->z);        // P->x = E = AA - BB = 4xz
    mul_word(P->z, P->x, A24); // P->z = E(a - 2)/4 = 4xz(a - 2)/4 = axz - 2xz
    add(P->z, P->z, t);        // P->z = E(a - 2)/4 + AA = xx + axz + zz
}

static void ladder_part2(struct xz *P, struct xz *Q, const fe_t t, const fe_t x)
{
    sqr1(Q->z);         // Q->z = (DA - CB)^2
    mul1(Q->z, x);      // Q->z = x(DA - CB)^2
    sqr1(Q->x);         // Q->x = (DA + CB)^2
    mul1(P->z, P->x);   // P->z = E(E(a - 2)/4 + AA)
    sub(P->x, t, P->x); // P->x = AA - E = AA - (AA - BB) = BB
    mul1(P->x, t);      // P->x = AABB
}

static void x25519_xz(struct xz *P, const unsigned char k[X25519_LEN],
                      const fe_t x)
{
    struct xz Q = {.z = {1}};
    memcpy(Q.x, x, sizeof(fe_t));
    memset(P, 0, sizeof *P);
    P->x[0] = 1;

    uint32_t swap = 0;
    for (int t = X25519_BITS - 1; t >= 0; --t)
    {
        const uint32_t kt = -(((uint32_t)k[t / 8] >> (t % 8)) & 1);
        cswap(swap ^ kt, P, &Q);
        swap = kt;
        fe_t t1;
        ladder_part1(P, &Q, t1);
        ladder_part2(P, &Q, t1, x);
    }
    cswap(swap, P, &Q);
}

static void xz_to_bytes(unsigned char out[X25519_LEN], struct xz *p)
{
    inv(p->z, p->z);
    mul1(p->x, p->z);
    canon(p->x);
    write_limbs(out, p->x);
}

void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char point[X25519_LEN])
{
    struct xz P;
    fe_t x;
    read_limbs(x, point);
    x25519_xz(&P, scalar, x);
    xz_to_bytes(out, &P);
}

#define BASE_POINT UINT32_C(9)

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    struct xz P;
    const fe_t B = {BASE_POINT};
    x25519_xz(&P, scalar, B);
    xz_to_bytes(out, &P);
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
    struct xz P, Q;
    fe_t A, B = {BASE_POINT};
    read_limbs(A, public_key);
    x25519_xz(&P, response, B);
    x25519_xz(&Q, challenge, A);
    // P = x/z = response*base_point
    // Q = u/w = challenge*public_key

    mul(A, Q.x, Q.z);
    mul_word(A, A, 16);
    // A = 16uw

    ladder_part1(&P, &Q, B);
    // Q.x = 2xu - 2zw
    // Q.z = 2zu - 2xw
    // P.z = xx + axz + zz

    read_limbs(B, public_nonce);
    // B = R

    mul1(P.z, A);
    mul1(P.z, B);
    // P.z = left = 16uwR(xx + axz + zz)

    mul1(Q.z, B);
    sub(Q.z, Q.z, Q.x);
    sqr1(Q.z);
    // Q.z = right = (R(2zu - 2xw) - (2xu - 2zw))^2

    // check equality
    sub(Q.z, Q.z, P.z);

    /*
     * If canon(Q.z) then the two sides are equal.
     * If canon(P.z) also, then both sides are zero.
     *
     * Reject signatures where both sides are zero, because that can happen if
     * an input causes the ladder to return 0/0.
     */
    return canon(Q.z) & ~canon(P.z);
}

#define MONTGOMERY_FACTOR UINT32_C(0x12547e1b)

/*
 * Set t = t + ab mod l
 */
static void sc_montmul(scalar_t t, const scalar_t a, const scalar_t b)
{
    /*
     * OK, so carry bounding. We're using a high carry, so that the inputs
     * don't have to be reduced.
     *
     * First montmul: output < (M^2 + Mp)/M = M+p, subtract p, < M. This gets
     * rid of high carry. Second montmul, by r^2 mod p < p: output < (Mp +
     * Mp)/M = 2p, subtract p, < p, done.
     */
    static const scalar_t sc_p = {
        0x5cf5d3edU, 0x5812631aU, 0xa2f79cd6U, 0x14def9deU,
        0x00000000U, 0x00000000U, 0x00000000U, 0x10000000U,
    };

    uint32_t hic = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        uint32_t carry = 0, carry2 = 0, mand = MONTGOMERY_FACTOR;

        for (int j = 0; j < NLIMBS; ++j)
        {
            uint32_t acc = t[j];
            acc = mac(&carry, acc, a[i], b[j]);
            if (j == 0)
                mand *= acc;
            acc = mac(&carry2, acc, mand, sc_p[j]);
            if (j > 0)
                t[j - 1] = acc;
        }

        /* Add two carry registers and high carry */
        t[NLIMBS - 1] = adc(&hic, carry, carry2);
    }

    /* Reduce */
    int64_t scarry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        scarry = scarry + t[i] - sc_p[i];
        t[i] = (uint32_t)scarry;
        scarry >>= WBITS;
    }
    uint32_t need_add = (uint32_t)(-(scarry + hic));

    uint32_t carry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        t[i] = mac(&carry, t[i], need_add, sc_p[i]);
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
    static const scalar_t sc_r2 = {
        0x449c0f01U, 0xa40611e3U, 0x68859347U, 0xd00e1ba7U,
        0x17f5be65U, 0xceec73d2U, 0x7c309a3dU, 0x0399411bU,
    };
    /* FUTURE memory/code size: just make secret_nonce non-const? */
    scalar_t scalar1, scalar2, scalar3;
    read_limbs(scalar1, secret_nonce);
    read_limbs(scalar2, secret_key);
    read_limbs(scalar3, challenge);
    sc_montmul(scalar1, scalar2, scalar3);
    memset(scalar2, 0, sizeof(scalar2));
    sc_montmul(scalar2, scalar1, sc_r2);
    write_limbs(response, scalar2);
}
