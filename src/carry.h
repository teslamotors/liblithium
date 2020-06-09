#ifndef LITHIUM_CARRY_H
#define LITHIUM_CARRY_H

#include <lithium/fe.h>

/*
 * Portable implementation of an arithmetic shift right on a signed double limb.
 * Used for shifting signed carry values to be added in to the next limb.
 * The portable implementation avoids arithmetic shift right on signed values
 * and casts from unsigned to signed values where the result would be negative,
 * as these operations are implementation-dependent.
 */
static sdlimb_t asr(sdlimb_t x, int b)
{
#if ((-5 >> 1 != -3) || defined(LITH_FORCE_PORTABLE_ASR))
    const dlimb_t ux = (dlimb_t)x;
    const dlimb_t sign = ux >> ((LITH_X25519_WBITS * 2) - 1); /* sign bit */
    const dlimb_t sign_mask = ~(sign - 1);
    const dlimb_t sign_extend = ~(((dlimb_t)-1) >> b);
    const dlimb_t pos = ux >> b;
    const dlimb_t neg = sign_extend | pos;
    return (sdlimb_t)(pos & ~sign_mask) - (sdlimb_t)((~neg + 1) & sign_mask);
#else
    return x >> b;
#endif
}

/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

/*
 * Multiply-accumulate with addend.
 */
static limb_t mac(limb_t *carry, limb_t a, limb_t b, limb_t c)
{
    const dlimb_t tmp = (dlimb_t)b * c + a + *carry;
    *carry = (limb_t)(tmp >> LITH_X25519_WBITS);
    return (limb_t)tmp;
}

/*
 * Add with carry.
 */
static limb_t adc(limb_t *carry, limb_t a, limb_t b)
{
    const dlimb_t total = (dlimb_t)a + b + *carry;
    *carry = (limb_t)(total >> LITH_X25519_WBITS);
    return (limb_t)total;
}

#endif /* LITHIUM_CARRY_H */
