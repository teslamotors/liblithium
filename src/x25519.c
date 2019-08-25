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
#define LIMB(x) ((uint32_t)UINT64_C(x)), ((uint32_t)(UINT64_C(x) >> 32))
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
    over = x[NLIMBS - 1] >> (WBITS - 1) | over << 1;
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

static void mul(fe_t out, const fe_t a, const fe_t b, int nb)
{
    /*
     * GCC at least produces pretty decent asm for this, so don't need to have
     * dedicated asm.
     */
    uint32_t accum[2 * NLIMBS] = {0};
    uint32_t carry2;

    for (int i = 0; i < nb; ++i)
    {
        carry2 = 0;
        uint32_t mand = b[i];
        for (int j = 0; j < NLIMBS; ++j)
        {
            accum[i + j] = mac(&carry2, accum[i + j], mand, a[j]);
        }
        accum[i + NLIMBS] = carry2;
    }

    carry2 = 0;
    const uint32_t mand = 38;
    for (int i = 0; i < NLIMBS; ++i)
    {
        out[i] = mac(&carry2, accum[i], mand, accum[i + NLIMBS]);
    }
    propagate(out, carry2);
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
     * is congruent to it mod 2^255-19.
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

static void ladder_part1(struct xz *p2, struct xz *p3, fe_t t1)
{
    static const uint32_t a24 = 121665;

    add(t1, p2->x, p2->z);      // t1 = A
    sub(p2->z, p2->x, p2->z);   // p2->z = B
    add(p2->x, p3->x, p3->z);   // p2->x = C
    sub(p3->z, p3->x, p3->z);   // p3->z = D
    mul1(p3->z, t1);            // p3->z = DA
    mul1(p2->x, p2->z);         // p3->x = BC
    add(p3->x, p3->z, p2->x);   // p3->x = DA+CB
    sub(p3->z, p3->z, p2->x);   // p3->z = DA-CB
    sqr1(t1);                   // t1 = AA
    sqr1(p2->z);                // p2->z = BB
    sub(p2->x, t1, p2->z);      // p2->x = E = AA-BB
    mul(p2->z, p2->x, &a24, 1); // p2->z = E*a24
    add(p2->z, p2->z, t1);      // p2->z = E*a24 + AA
}

static void ladder_part2(struct xz *p2, struct xz *p3, const fe_t t1,
                         const fe_t x1)
{
    sqr1(p3->z);           // p3->z = (DA-CB)^2
    mul1(p3->z, x1);       // p3->z = x1 * (DA-CB)^2
    sqr1(p3->x);           // p3->x = (DA+CB)^2
    mul1(p2->z, p2->x);    // p2->z = AA*(E*a24+AA)
    sub(p2->x, t1, p2->x); // p2->x = BB again
    mul1(p2->x, t1);       // p2->x = AA*BB
}

static void x25519_xz(struct xz *p2, const unsigned char k[X25519_LEN],
                      struct xz *p3)
{
    fe_t x1;
    memcpy(x1, p3->x, sizeof(fe_t));
    memset(p2, 0, sizeof *p2);
    p2->x[0] = 1;

    uint32_t swap = 0;
    for (int t = 255; t >= 0; --t)
    {
        const uint32_t kt = -(((uint32_t)k[t / 8] >> (t % 8)) & 1);
        cswap(swap ^ kt, p2, p3);
        swap = kt;
        fe_t t1;
        ladder_part1(p2, p3, t1);
        ladder_part2(p2, p3, t1, x1);
    }
    cswap(swap, p2, p3);
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
    struct xz p2, p3 = {.z = {1}};
    read_limbs(p3.x, point);
    x25519_xz(&p2, scalar, &p3);
    xz_to_bytes(out, &p2);
}

static const struct xz base_point = {.x = {9}, .z = {1}};

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    struct xz p2, p3 = base_point;
    x25519_xz(&p2, scalar, &p3);
    xz_to_bytes(out, &p2);
}

int x25519_verify_p2(const unsigned char response[X25519_LEN],
                     const unsigned char challenge[X25519_LEN],
                     const unsigned char eph[X25519_LEN],
                     const unsigned char pub[X25519_LEN])
{
    struct xz hA, sB, p2 = {.z = {1}}, p3 = base_point;
    read_limbs(p2.x, pub);
    x25519_xz(&hA, challenge, &p2);
    x25519_xz(&sB, response, &p3);

    read_limbs(p2.x, eph);

    memcpy(&p3, &hA, sizeof(p3));

    ladder_part1(&sB, &p3, p2.z);

    mul1(sB.z, hA.x);
    mul1(sB.z, hA.z);
    mul1(sB.z, p2.x);
    const uint32_t sixteen = 16;
    mul(sB.z, sB.z, &sixteen, 1);

    mul1(p3.z, p2.x);
    sub(p3.z, p3.z, p3.x);
    sqr1(p3.z);

    /* check equality */
    sub(p3.z, p3.z, sB.z);

    /*
     * If canon(sB.z) then both sides are zero.
     * If canon(z3) then the two sides are equal.
     *
     * Reject sigs where both sides are zero, because that can happen if an
     * input causes the ladder to return 0/0.
     */
    return (int)(canon(sB.z) | ~canon(p3.z));
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
        LIMB(0x5812631a5cf5d3ed),
        LIMB(0x14def9dea2f79cd6),
        LIMB(0x0000000000000000),
        LIMB(0x1000000000000000),
    };
    static const uint32_t MONTGOMERY_FACTOR =
        (uint32_t)UINT64_C(0xd2b51da312547e1b);

    uint32_t hic = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        uint32_t carry = 0, carry2 = 0, mand = a[i], mand2 = MONTGOMERY_FACTOR;

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

void x25519_sign_p2(unsigned char response[X25519_LEN],
                    const unsigned char challenge[X25519_LEN],
                    const unsigned char eph_secret[X25519_LEN],
                    const unsigned char secret[X25519_LEN])
{
    static const scalar_t sc_r2 = {
        LIMB(0xa40611e3449c0f01),
        LIMB(0xd00e1ba768859347),
        LIMB(0xceec73d217f5be65),
        LIMB(0x0399411b7c309a3d),
    };
    /* FUTURE memory/code size: just make eph_secret non-const? */
    scalar_t scalar1, scalar2, scalar3;
    read_limbs(scalar1, eph_secret);
    read_limbs(scalar2, secret);
    read_limbs(scalar3, challenge);
    sc_montmul(scalar1, scalar2, scalar3);
    memset(scalar2, 0, sizeof(scalar2));
    sc_montmul(scalar2, scalar1, sc_r2);
    write_limbs(response, scalar2);
}
