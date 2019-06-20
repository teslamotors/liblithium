/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#include <lithium/x25519.h>

#include <stdint.h>
#include <string.h>

#define X25519_WBITS 32
typedef uint32_t uint32_t;
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
#define LIMB(x) ((uint32_t)UINT64_C(x)), ((uint32_t)(UINT64_C(x) >> 32))

#define X25519_WOCTETS (X25519_WBITS / 8)

#define NLIMBS (256 / X25519_WBITS)

static void read_limbs(uint32_t x[NLIMBS], const unsigned char *in)
{
    unsigned i;
    for (i = 0; i < NLIMBS; i++)
    {
        x[i] = read_limb(in + i * X25519_WOCTETS);
    }
}

static void write_limbs(unsigned char *out, const uint32_t x[NLIMBS])
{
    unsigned i;
    for (i = 0; i < NLIMBS; i++)
    {
        write_limb(out + i * X25519_WOCTETS, x[i]);
    }
}

typedef uint32_t fe_t[NLIMBS];
typedef uint32_t scalar_t[NLIMBS];

static const uint32_t MONTGOMERY_FACTOR =
    (uint32_t)UINT64_C(0xd2b51da312547e1b);
static const scalar_t sc_p = {LIMB(0x5812631a5cf5d3ed),
                              LIMB(0x14def9dea2f79cd6),
                              LIMB(0x0000000000000000),
                              LIMB(0x1000000000000000)},
                      sc_r2 = {
                          LIMB(0xa40611e3449c0f01), LIMB(0xd00e1ba768859347),
                          LIMB(0xceec73d217f5be65), LIMB(0x0399411b7c309a3d)};

static uint32_t umaal(uint32_t *carry, uint32_t acc, uint32_t mand,
                      uint32_t mier)
{
    uint64_t tmp = (uint64_t)mand * mier + acc + *carry;
    *carry = (uint32_t)(tmp >> X25519_WBITS);
    return (uint32_t)tmp;
}

/* These functions are implemented in terms of umaal on ARM */
static uint32_t adc(uint32_t *carry, uint32_t acc, uint32_t mand)
{
    uint64_t total = (uint64_t)*carry + acc + mand;
    *carry = (uint32_t)(total >> X25519_WBITS);
    return (uint32_t)total;
}

static uint32_t adc0(uint32_t *carry, uint32_t acc)
{
    uint64_t total = (uint64_t)*carry + acc;
    *carry = (uint32_t)(total >> X25519_WBITS);
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
    over = x[NLIMBS - 1] >> (X25519_WBITS - 1) | over << 1;
    x[NLIMBS - 1] &= ~((uint32_t)1 << (X25519_WBITS - 1));

    uint32_t carry = over * 19;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        x[i] = adc0(&carry, x[i]);
    }
}

static void add(fe_t out, const fe_t a, const fe_t b)
{
    uint32_t carry = 0;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        out[i] = adc(&carry, a[i], b[i]);
    }
    propagate(out, carry);
}

static void sub(fe_t out, const fe_t a, const fe_t b)
{
    int64_t carry = -38;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        carry = carry + a[i] - b[i];
        out[i] = (uint32_t)carry;
        carry >>= X25519_WBITS;
    }
    propagate(out, (uint32_t)(1 + carry));
}

static void mul(fe_t out, const fe_t a, const fe_t b, unsigned nb)
{
    /*
     * GCC at least produces pretty decent asm for this, so don't need to have
     * dedicated asm.
     */
    uint32_t accum[2 * NLIMBS] = {0};
    uint32_t carry2;

    for (unsigned i = 0; i < nb; i++)
    {
        carry2 = 0;
        uint32_t mand = b[i];
        for (unsigned j = 0; j < NLIMBS; j++)
        {
            accum[i + j] = umaal(&carry2, accum[i + j], mand, a[j]);
        }
        accum[i + NLIMBS] = carry2;
    }

    carry2 = 0;
    const uint32_t mand = 38;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        out[i] = umaal(&carry2, accum[i], mand, accum[i + NLIMBS]);
    }
    propagate(out, carry2);
}

static void sqr(fe_t out, const fe_t a)
{
    mul(out, a, a, NLIMBS);
}

static void mul1(fe_t out, const fe_t a)
{
    mul(out, a, out, NLIMBS);
}

static void sqr1(fe_t a)
{
    mul1(a, a);
}

static void condswap(uint32_t a[2 * NLIMBS], uint32_t b[2 * NLIMBS],
                     uint32_t doswap)
{
    for (unsigned i = 0; i < 2 * NLIMBS; i++)
    {
        uint32_t xor = (a[i] ^ b[i]) & doswap;
        a[i] ^= xor;
        b[i] ^= xor;
    }
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
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        x[i] = adc0(&carry0, x[i]);
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
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        carry += x[i];
        x[i] = (uint32_t)carry;
        res |= x[i];
        carry >>= X25519_WBITS;
    }
    return (uint32_t)(((uint64_t)res - 1) >> X25519_WBITS);
}

static const uint32_t a24[1] = {121665};

static void ladder_part1(fe_t xs[5])
{
    uint32_t *x2 = xs[0], *z2 = xs[1], *x3 = xs[2], *z3 = xs[3], *t1 = xs[4];
    add(t1, x2, z2);                                // t1 = A
    sub(z2, x2, z2);                                // z2 = B
    add(x2, x3, z3);                                // x2 = C
    sub(z3, x3, z3);                                // z3 = D
    mul1(z3, t1);                                   // z3 = DA
    mul1(x2, z2);                                   // x3 = BC
    add(x3, z3, x2);                                // x3 = DA+CB
    sub(z3, z3, x2);                                // z3 = DA-CB
    sqr1(t1);                                       // t1 = AA
    sqr1(z2);                                       // z2 = BB
    sub(x2, t1, z2);                                // x2 = E = AA-BB
    mul(z2, x2, a24, sizeof(a24) / sizeof(a24[0])); // z2 = E*a24
    add(z2, z2, t1);                                // z2 = E*a24 + AA
}

static void ladder_part2(fe_t xs[5], const fe_t x1)
{
    uint32_t *x2 = xs[0], *z2 = xs[1], *x3 = xs[2], *z3 = xs[3], *t1 = xs[4];
    sqr1(z3);        // z3 = (DA-CB)^2
    mul1(z3, x1);    // z3 = x1 * (DA-CB)^2
    sqr1(x3);        // x3 = (DA+CB)^2
    mul1(z2, x2);    // z2 = AA*(E*a24+AA)
    sub(x2, t1, x2); // x2 = BB again
    mul1(x2, t1);    // x2 = AA*BB
}

static void x25519_core(fe_t xs[5], const unsigned char scalar[X25519_LEN],
                        const unsigned char base[X25519_LEN])
{
    fe_t x1;
    read_limbs(x1, base);

    uint32_t swap = 0;
    uint32_t *x2 = xs[0], *x3 = xs[2], *z3 = xs[3];
    memset(xs, 0, 4 * sizeof(fe_t));
    x2[0] = z3[0] = 1;
    memcpy(x3, x1, sizeof(fe_t));

    for (int i = 255; i >= 0; --i)
    {
        uint32_t doswap = -(uint32_t)((scalar[i / 8] >> (i % 8)) & 1);
        condswap(x2, x3, swap ^ doswap);
        swap = doswap;

        ladder_part1(xs);
        ladder_part2(xs, x1);
    }

    condswap(x2, x3, swap);
}

void x25519(unsigned char out[X25519_LEN],
            const unsigned char scalar[X25519_LEN],
            const unsigned char base[X25519_LEN])
{
    fe_t xs[5];
    x25519_core(xs, scalar, base);

    /* Precomputed inversion chain */
    uint32_t *x2 = xs[0], *z2 = xs[1], *z3 = xs[3];

    uint32_t *prev = z2;
    /* Raise to the p-2 = 0x7f..ffeb */
    for (int i = 253; i >= 0; i--)
    {
        sqr(z3, prev);
        prev = z3;
        if (i >= 8 || (0xeb >> i & 1))
        {
            mul1(z3, z2);
        }
    }

    /* Here prev = z3 */
    /* x2 /= z2 */
    mul1(x2, z3);
    canon(x2);
    write_limbs(out, x2);
}

static const unsigned char base_point[X25519_LEN] = {9};

void x25519_base(unsigned char out[X25519_LEN],
                 const unsigned char scalar[X25519_LEN])
{
    x25519(out, scalar, base_point);
}

static uint32_t x25519_verify_core(fe_t xs[5], const uint32_t *other1,
                                   const unsigned char other2[X25519_LEN])
{
    uint32_t *z2 = xs[1], *x3 = xs[2], *z3 = xs[3];

    fe_t xo2;
    read_limbs(xo2, other2);

    memcpy(x3, other1, 2 * sizeof(fe_t));

    ladder_part1(xs);

    /* Here z2 = t2^2 */
    mul1(z2, other1);
    mul1(z2, other1 + NLIMBS);
    mul1(z2, xo2);
    const uint32_t sixteen = 16;
    mul(z2, z2, &sixteen, 1);

    mul1(z3, xo2);
    sub(z3, z3, x3);
    sqr1(z3);

    /* check equality */
    sub(z3, z3, z2);

    /*
     * If canon(z2) then both sides are zero.
     * If canon(z3) then the two sides are equal.
     *
     * Reject sigs where both sides are zero, because that can happen if an
     * input causes the ladder to return 0/0.
     */
    return canon(z2) | ~canon(z3);
}

int x25519_verify_p2(const unsigned char response[X25519_LEN],
                     const unsigned char challenge[X25519_LEN],
                     const unsigned char eph[X25519_LEN],
                     const unsigned char pub[X25519_LEN])
{
    fe_t xs[7];
    x25519_core(&xs[0], challenge, pub);
    x25519_core(&xs[2], response, base_point);
    return (int)x25519_verify_core(&xs[2], xs[0], eph);
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
    uint32_t hic = 0;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        uint32_t carry = 0, carry2 = 0, mand = a[i], mand2 = MONTGOMERY_FACTOR;

        for (unsigned j = 0; j < NLIMBS; j++)
        {
            uint32_t acc = out[j];
            acc = umaal(&carry, acc, mand, b[j]);
            if (j == 0)
                mand2 *= acc;
            acc = umaal(&carry2, acc, mand2, sc_p[j]);
            if (j > 0)
                out[j - 1] = acc;
        }

        /* Add two carry registers and high carry */
        out[NLIMBS - 1] = adc(&hic, carry, carry2);
    }

    /* Reduce */
    int64_t scarry = 0;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        scarry = scarry + out[i] - sc_p[i];
        out[i] = (uint32_t)scarry;
        scarry >>= X25519_WBITS;
    }
    uint32_t need_add = (uint32_t)(-(scarry + hic));

    uint32_t carry = 0;
    for (unsigned i = 0; i < NLIMBS; i++)
    {
        out[i] = umaal(&carry, out[i], need_add, sc_p[i]);
    }
}

void x25519_sign_p2(unsigned char response[X25519_LEN],
                    const unsigned char challenge[X25519_LEN],
                    const unsigned char eph_secret[X25519_LEN],
                    const unsigned char secret[X25519_LEN])
{
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
