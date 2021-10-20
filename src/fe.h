/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's x25519.c, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#ifndef LITHIUM_FE_H
#define LITHIUM_FE_H

#include <lithium/x25519.h>

#include <stdint.h>

#ifndef LITH_X25519_WBITS
#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
#define LITH_X25519_WBITS 64
#else
#define LITH_X25519_WBITS 32
#endif
#endif

#if (LITH_X25519_WBITS == 16)

typedef uint16_t limb;
typedef uint32_t dlimb;
typedef int32_t sdlimb;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    UINT16_C(lsw), UINT16_C(slsw), UINT16_C(smsw), UINT16_C(msw)
#define LOW_LIMB(lsw, slsw, smsw, msw) UINT16_C(lsw)

#elif (LITH_X25519_WBITS == 32)

typedef uint32_t limb;
typedef uint64_t dlimb;
typedef int64_t sdlimb;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    (UINT32_C(lsw) | (UINT32_C(slsw) << 16)),                                  \
        (UINT32_C(smsw) | (UINT32_C(msw) << 16))
#define LOW_LIMB(lsw, slsw, smsw, msw) (UINT32_C(lsw) | (UINT32_C(slsw) << 16))

#elif (LITH_X25519_WBITS == 64)

typedef uint64_t limb;
typedef __uint128_t dlimb;
typedef __int128_t sdlimb;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    (UINT64_C(lsw) | (UINT64_C(slsw) << 16) | (UINT64_C(smsw) << 32) |         \
     (UINT64_C(msw) << 48))
#define LOW_LIMB LIMBS

#else
#error "Unsupported value for LITH_X25519_WBITS"
#endif

#define NLIMBS (X25519_BITS / LITH_X25519_WBITS)

#define LIMB_HIGH_BIT_MASK ((limb)1U << (LITH_X25519_WBITS - 1))

typedef limb fe[NLIMBS];

void read_limbs(limb x[NLIMBS], const unsigned char *in);

void write_limbs(unsigned char *out, const limb x[NLIMBS]);

void add(fe out, const fe a, const fe b);

void sub(fe out, const fe a, const fe b);

void mul(fe out, const fe a, const fe b);

void mul1(fe a, const fe b);

void mul_word(fe out, const fe a, limb b);

void sqr1(fe a);

limb canon(fe a);

void inv(fe a);

sdlimb asr(sdlimb x, int b);

limb mac(limb *carry, limb a, limb b, limb c);

limb adc(limb *carry, limb a, limb b);

#endif /* LITHIUM_FE_H */
