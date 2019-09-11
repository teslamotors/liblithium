/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#include <lithium/x25519.h>

#include <stdint.h>
#include <string.h>

#define WBITS 32
#define WLEN (WBITS / 8)
#define NLIMBS (X25519_BITS / WBITS)

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

typedef uint32_t fe_t[NLIMBS];
typedef uint32_t scalar_t[NLIMBS];

static uint32_t mac(uint32_t *carry, uint32_t a, uint32_t b, uint32_t c)
{
    uint64_t tmp = (uint64_t)b * c + a + *carry;
    *carry = (uint32_t)(tmp >> WBITS);
    return (uint32_t)tmp;
}

static uint32_t adc(uint32_t *carry, uint32_t a, uint32_t b)
{
    uint64_t total = (uint64_t)a + b + *carry;
    *carry = (uint32_t)(total >> WBITS);
    return (uint32_t)total;
}

/*
 * Precondition: carry is small.
 * Invariant: result of propagate is < 2^255 + 1 word
 * In particular, always less than 2p.
 * Also, output x >= min(x,19)
 */
static void propagate(fe_t x, uint32_t over)
{
    over = (over << 1) | (x[NLIMBS - 1] >> (WBITS - 1));
    x[NLIMBS - 1] &= ~((uint32_t)1 << (WBITS - 1));

    uint32_t carry = over * 19;
    for (int i = 0; i < NLIMBS; ++i)
    {
        x[i] = adc(&carry, x[i], 0);
    }
}

static void add(fe_t out, const fe_t a, const fe_t b)
{
    uint32_t carry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        out[i] = adc(&carry, a[i], b[i]);
    }
    propagate(out, carry);
}

static void sub(fe_t out, const fe_t a, const fe_t b)
{
    int64_t carry = -38;
    for (int i = 0; i < NLIMBS; ++i)
    {
        carry = carry + a[i] - b[i];
        out[i] = (uint32_t)carry;
        carry >>= WBITS;
    }
    propagate(out, (uint32_t)(1 + carry));
}

static void mul(fe_t out, const fe_t a, const uint32_t *b, int nb)
{
    uint32_t accum[NLIMBS * 2] = {0};
    uint32_t carry;

    for (int i = 0; i < nb; ++i)
    {
        carry = 0;
        for (int j = 0; j < NLIMBS; ++j)
        {
            accum[i + j] = mac(&carry, accum[i + j], b[i], a[j]);
        }
        accum[i + NLIMBS] = carry;
    }

    carry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        out[i] = mac(&carry, accum[i], 38, accum[i + NLIMBS]);
    }
    propagate(out, carry);
}

static void mul1(fe_t out, const fe_t a)
{
    mul(out, a, out, NLIMBS);
}

static void sqr1(fe_t a)
{
    mul1(a, a);
}

static uint32_t canon(fe_t x)
{
    /*
     * Canonicalize a field element x, reducing it to the least residue which
     * is congruent to it mod 2^255-19. Returns 0 if the residue is nonzero.
     *
     * Precondition: x < 2^255 + 1 word
     */

    /* First, add 19. */
    uint32_t carry0 = 19;
    for (int i = 0; i < NLIMBS; ++i)
    {
        x[i] = adc(&carry0, x[i], 0);
    }
    propagate(x, carry0);

    /*
     * Here, 19 <= x2 < 2^255
     *
     * This is because we added 19, so before propagate it can't be less than
     * 19. After propagate, it still can't be less than 19, because if
     * propagate does anything it adds 19.
     *
     * We know that the high bit must be clear, because either the input was
     * ~2^255 + one word + 19 (in which case it propagates to at most 2 words)
     * or it was < 2^255.
     *
     * So now, if we subtract 19, we will get back to something in [0,2^255-19).
     */
    int64_t carry = -19;
    uint32_t res = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        carry += x[i];
        x[i] = (uint32_t)carry;
        res |= x[i];
        carry >>= WBITS;
    }
    return (uint32_t)(((uint64_t)res - 1) >> WBITS);
}

static void inv(fe_t out, const fe_t a)
{
    fe_t t = {1};
    /* Raise to the p-2 = 0x7f..ffeb */
    for (int i = 254; i >= 0; --i)
    {
        sqr1(t);
        if (i >= 8 || ((0xeb >> i) & 1))
        {
            mul1(t, a);
        }
    }
    memcpy(out, t, sizeof(fe_t));
}

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

static void ladder_part1(struct xz *P, struct xz *Q, fe_t t1)
{
    static const uint32_t a24 = 121665;

    add(t1, P->x, P->z);      // t1 = A
    sub(P->z, P->x, P->z);    // P->z = B
    add(P->x, Q->x, Q->z);    // P->x = C
    sub(Q->z, Q->x, Q->z);    // Q->z = D
    mul1(Q->z, t1);           // Q->z = DA
    mul1(P->x, P->z);         // Q->x = BC
    add(Q->x, Q->z, P->x);    // Q->x = DA+CB
    sub(Q->z, Q->z, P->x);    // Q->z = DA-CB
    sqr1(t1);                 // t1 = AA
    sqr1(P->z);               // P->z = BB
    sub(P->x, t1, P->z);      // P->x = E = AA-BB
    mul(P->z, P->x, &a24, 1); // P->z = E*a24
    add(P->z, P->z, t1);      // P->z = E*a24 + AA
}

static void ladder_part2(struct xz *P, struct xz *Q, const fe_t t1,
                         const fe_t x1)
{
    sqr1(Q->z);          // Q->z = (DA-CB)^2
    mul1(Q->z, x1);      // Q->z = x1 * (DA-CB)^2
    sqr1(Q->x);          // Q->x = (DA+CB)^2
    mul1(P->z, P->x);    // P->z = AA*(E*a24+AA)
    sub(P->x, t1, P->x); // P->x = BB again
    mul1(P->x, t1);      // P->x = AA*BB
}

static void x25519_xz(struct xz *P, const unsigned char k[X25519_LEN],
                      const fe_t x1)
{
    struct xz Q = {.z = {1}};
    memcpy(Q.x, x1, sizeof(fe_t));
    memset(P, 0, sizeof *P);
    P->x[0] = 1;

    uint32_t swap = 0;
    for (int t = 255; t >= 0; --t)
    {
        const uint32_t kt = -(((uint32_t)k[t / 8] >> (t % 8)) & 1);
        cswap(swap ^ kt, P, &Q);
        swap = kt;
        fe_t t1;
        ladder_part1(P, &Q, t1);
        ladder_part2(P, &Q, t1, x1);
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
    fe_t x1;
    read_limbs(x1, point);
    x25519_xz(&P, scalar, x1);
    xz_to_bytes(out, &P);
}

#define BASE_POINT 9

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    struct xz P;
    fe_t B = {BASE_POINT};
    x25519_xz(&P, scalar, B);
    xz_to_bytes(out, &P);
}

bool x25519_verify(const unsigned char response[X25519_LEN],
                   const unsigned char challenge[X25519_LEN],
                   const unsigned char public_nonce[X25519_LEN],
                   const unsigned char public_key[X25519_LEN])
{
    struct xz P, Q;
    fe_t A, B = {BASE_POINT};
    read_limbs(A, public_key);
    x25519_xz(&P, response, B);
    x25519_xz(&Q, challenge, A);

    mul(A, Q.x, Q.z, NLIMBS);
    const uint32_t sixteen = 16;
    mul(A, A, &sixteen, 1);

    ladder_part1(&P, &Q, B);

    read_limbs(B, public_nonce);
    mul1(P.z, A);
    mul1(P.z, B);

    mul1(Q.z, B);
    sub(Q.z, Q.z, Q.x);
    sqr1(Q.z);

    /* check equality */
    sub(Q.z, Q.z, P.z);

    /*
     * If canon(Q.z) then the two sides are equal.
     * If canon(P.z) then both sides are zero.
     *
     * Reject sigs where both sides are zero, because that can happen if an
     * input causes the ladder to return 0/0.
     */
    return canon(Q.z) & ~canon(P.z);
}

static void sc_montmul(scalar_t out, const scalar_t a, const scalar_t b)
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
    static const uint32_t montgomery_factor = 0x12547e1bU;

    uint32_t hic = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        uint32_t carry = 0, carry2 = 0, mand = a[i], mand2 = montgomery_factor;

        for (int j = 0; j < NLIMBS; ++j)
        {
            uint32_t acc = out[j];
            acc = mac(&carry, acc, mand, b[j]);
            if (j == 0)
                mand2 *= acc;
            acc = mac(&carry2, acc, mand2, sc_p[j]);
            if (j > 0)
                out[j - 1] = acc;
        }

        /* Add two carry registers and high carry */
        out[NLIMBS - 1] = adc(&hic, carry, carry2);
    }

    /* Reduce */
    int64_t scarry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        scarry = scarry + out[i] - sc_p[i];
        out[i] = (uint32_t)scarry;
        scarry >>= WBITS;
    }
    uint32_t need_add = (uint32_t)(-(scarry + hic));

    uint32_t carry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        out[i] = mac(&carry, out[i], need_add, sc_p[i]);
    }
}

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
