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

typedef uint16_t limb_t;
typedef uint32_t dlimb_t;
typedef int32_t sdlimb_t;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    UINT16_C(lsw), UINT16_C(slsw), UINT16_C(smsw), UINT16_C(msw)
#define LOW_LIMB(lsw, slsw, smsw, msw) UINT16_C(lsw)

#elif (LITH_X25519_WBITS == 32)

typedef uint32_t limb_t;
typedef uint64_t dlimb_t;
typedef int64_t sdlimb_t;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    (UINT32_C(lsw) | (UINT32_C(slsw) << 16)),                                  \
        (UINT32_C(smsw) | (UINT32_C(msw) << 16))
#define LOW_LIMB(lsw, slsw, smsw, msw) (UINT32_C(lsw) | (UINT32_C(slsw) << 16))

#elif (LITH_X25519_WBITS == 64)

typedef uint64_t limb_t;
typedef __uint128_t dlimb_t;
typedef __int128_t sdlimb_t;
#define LIMBS(lsw, slsw, smsw, msw)                                            \
    (UINT64_C(lsw) | (UINT64_C(slsw) << 16) | (UINT64_C(smsw) << 32) |         \
     (UINT64_C(msw) << 48))
#define LOW_LIMB LIMBS

#else
#error "Unsupported value for LITH_X25519_WBITS"
#endif

#define NLIMBS (X25519_BITS / LITH_X25519_WBITS)

typedef limb_t fe_t[NLIMBS];

void read_limbs(limb_t x[NLIMBS], const unsigned char *in);

void write_limbs(unsigned char *out, const limb_t x[NLIMBS]);

void add(fe_t out, const fe_t a, const fe_t b);

void sub(fe_t out, const fe_t a, const fe_t b);

void mul(fe_t out, const fe_t a, const fe_t b);

void mul1(fe_t a, const fe_t b);

void mul_word(fe_t out, const fe_t a, limb_t b);

void sqr1(fe_t a);

limb_t canon(fe_t a);

void inv(fe_t out, const fe_t a);

#endif /* LITHIUM_FE_H */
