#include <lithium/fe.h>

#include "bytes.h"
#include "carry.h"

#include <string.h>

#define WLEN (WBITS / 8)

void read_limbs(uint32_t x[NLIMBS], const unsigned char *in)
{
    for (int i = 0; i < NLIMBS; ++i)
    {
        x[i] = bytes_to_u32(in + i * WLEN);
    }
}

void write_limbs(unsigned char *out, const uint32_t x[NLIMBS])
{
    for (int i = 0; i < NLIMBS; ++i)
    {
        bytes_from_u32(out + i * WLEN, x[i]);
    }
}

/*
 * Precondition: carry is small.
 * Invariant: result of propagate is < 2^255 + 1 word
 * In particular, always less than 2p.
 * Also, output x >= min(x,19)
 */
static void propagate(fe_t x, uint32_t carry)
{
    carry = ((carry << 1) | (x[NLIMBS - 1] >> (WBITS - 1))) * 19;
    x[NLIMBS - 1] &= ~((uint32_t)1 << (WBITS - 1));
    for (int i = 0; i < NLIMBS; ++i)
    {
        x[i] = adc(&carry, x[i], 0);
    }
}

void add(fe_t out, const fe_t a, const fe_t b)
{
    uint32_t carry = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        out[i] = adc(&carry, a[i], b[i]);
    }
    propagate(out, carry);
}

void sub(fe_t out, const fe_t a, const fe_t b)
{
    int64_t carry = -76;
    for (int i = 0; i < NLIMBS; ++i)
    {
        carry = carry + a[i] - b[i];
        out[i] = (uint32_t)carry;
        carry >>= WBITS;
    }
    propagate(out, (uint32_t)(carry + 2));
}

static void mul_n(fe_t out, const fe_t a, const uint32_t *b, int nb)
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

void mul(fe_t out, const fe_t a, const fe_t b)
{
    mul_n(out, a, b, NLIMBS);
}

void mul_word(fe_t out, const fe_t a, uint32_t b)
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

uint32_t canon(fe_t a)
{
    /*
     * Canonicalize a field element a, reducing it to the least residue which
     * is congruent to it mod 2^255-19. Returns 0 if the residue is nonzero.
     *
     * Precondition: x < 2^255 + 1 word
     */

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
    int64_t carry = -19;
    uint32_t res = 0;
    for (int i = 0; i < NLIMBS; ++i)
    {
        carry += a[i];
        a[i] = (uint32_t)carry;
        res |= a[i];
        carry >>= WBITS;
    }
    return (uint32_t)(((uint64_t)res - 1) >> WBITS);
}

void inv(fe_t out, const fe_t a)
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
