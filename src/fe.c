/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#include <lithium/fe.h>

#include "carry.h"

#include <string.h>

#define WLEN (LITH_X25519_WBITS / 8)

void read_limbs(limb_t x[NLIMBS], const unsigned char *in)
{
    int i, j;
    for (i = 0; i < NLIMBS; ++i)
    {
        x[i] = 0;
        for (j = 0; j < WLEN; ++j)
        {
            x[i] |= (limb_t)in[(i * WLEN) + j] << (j * 8);
        }
    }
}

void write_limbs(unsigned char *out, const limb_t x[NLIMBS])
{
    int i, j;
    for (i = 0; i < NLIMBS; ++i)
    {
        limb_t w = x[i];
        for (j = 0; j < WLEN; ++j)
        {
            out[(i * WLEN) + j] = (w >> (j * 8)) & 0xFFU;
        }
    }
}

/*
 * Precondition: carry is small.
 * Invariant: result of propagate is < 2^255 + 1 word
 * In particular, always less than 2p.
 * Also, output x >= min(x,19)
 */
static void propagate(fe_t x, limb_t carry)
{
    int i;
    carry <<= 1;
    carry |= x[NLIMBS - 1] >> (LITH_X25519_WBITS - 1);
    carry *= 19;
    x[NLIMBS - 1] &= ~((limb_t)1 << (LITH_X25519_WBITS - 1));
    for (i = 0; i < NLIMBS; ++i)
    {
        x[i] = adc(&carry, x[i], 0);
    }
}

void add(fe_t out, const fe_t a, const fe_t b)
{
    limb_t carry = 0;
    int i;
    for (i = 0; i < NLIMBS; ++i)
    {
        out[i] = adc(&carry, a[i], b[i]);
    }
    propagate(out, carry);
}

void sub(fe_t out, const fe_t a, const fe_t b)
{
    sdlimb_t carry = -76;
    int i;
    for (i = 0; i < NLIMBS; ++i)
    {
        carry = carry + a[i] - b[i];
        out[i] = (limb_t)carry;
        carry >>= LITH_X25519_WBITS;
    }
    propagate(out, (limb_t)(carry + 2));
}

static void mul_n(fe_t out, const fe_t a, const limb_t *b, int nb)
{
    limb_t accum[NLIMBS * 2] = {0};
    limb_t carry;

    int i, j;
    for (i = 0; i < nb; ++i)
    {
        carry = 0;
        for (j = 0; j < NLIMBS; ++j)
        {
            accum[i + j] = mac(&carry, accum[i + j], b[i], a[j]);
        }
        accum[i + NLIMBS] = carry;
    }

    carry = 0;
    for (i = 0; i < NLIMBS; ++i)
    {
        out[i] = mac(&carry, accum[i], 38, accum[i + NLIMBS]);
    }
    propagate(out, carry);
}

void mul(fe_t out, const fe_t a, const fe_t b)
{
    mul_n(out, a, b, NLIMBS);
}

void mul_word(fe_t out, const fe_t a, limb_t b)
{
    mul_n(out, a, &b, 1);
}

void mul1(fe_t a, const fe_t b)
{
    mul(a, b, a);
}

void sqr1(fe_t a)
{
    mul1(a, a);
}

limb_t canon(fe_t a)
{
    /*
     * Canonicalize a field element a, reducing it to the least residue which
     * is congruent to it mod 2^255-19. Returns 0 if the residue is nonzero.
     *
     * Precondition: x < 2^255 + 1 word
     */
    sdlimb_t carry;
    limb_t res;
    int i;

    /* First, add 19. */
    const fe_t nineteen = {19};
    add(a, a, nineteen);

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
    carry = -19;
    res = 0;
    for (i = 0; i < NLIMBS; ++i)
    {
        carry += a[i];
        a[i] = (limb_t)carry;
        res |= a[i];
        carry >>= LITH_X25519_WBITS;
    }
    return (limb_t)(((dlimb_t)res - 1) >> LITH_X25519_WBITS);
}

void inv(fe_t out, const fe_t a)
{
    fe_t t = {1};
    int i;
    /* Raise to the p-2 = 0x7f..ffeb */
    for (i = 254; i >= 0; --i)
    {
        sqr1(t);
        if (i >= 8 || ((0xeb >> i) & 1))
        {
            mul1(t, a);
        }
    }
    (void)memcpy(out, t, sizeof(fe_t));
}
