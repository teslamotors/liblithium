#ifndef LITHIUM_CARRY_H
#define LITHIUM_CARRY_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's x25519.c, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <lithium/fe.h>

/*
 * Portable implementation of an arithmetic shift right on a signed double limb.
 * Used for shifting signed carry values to be added in to the next limb.
 * The portable implementation avoids arithmetic shift right on signed values
 * and casts from unsigned to signed values where the result would be negative,
 * as these operations are implementation-dependent.
 */
static sdlimb asr(sdlimb x, int b)
{
#if ((-5 >> 1 != -3) || defined(LITH_FORCE_PORTABLE_ASR))
    const dlimb ux = (dlimb)x;
    const dlimb sign = ux >> ((LITH_X25519_WBITS * 2) - 1); /* sign bit */
    const dlimb sign_mask = ~(sign - 1);
    const dlimb sign_extend = ~(((dlimb)-1) >> b);
    const dlimb pos = ux >> b;
    const dlimb neg = sign_extend | pos;
    return (sdlimb)(pos & ~sign_mask) - (sdlimb)((~neg + 1) & sign_mask);
#else
    return x >> b;
#endif
}

/*
 * Multiply-accumulate with addend.
 */
static limb mac(limb *carry, limb a, limb b, limb c)
{
    const dlimb tmp = (dlimb)b * c + a + *carry;
    *carry = (limb)(tmp >> LITH_X25519_WBITS);
    return (limb)tmp;
}

/*
 * Add with carry.
 */
static limb adc(limb *carry, limb a, limb b)
{
    const dlimb total = (dlimb)a + b + *carry;
    *carry = (limb)(total >> LITH_X25519_WBITS);
    return (limb)total;
}

#endif /* LITHIUM_CARRY_H */
