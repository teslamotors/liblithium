/*
 * Copyright (c) 2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#ifndef LITHIUM_FE_H
#define LITHIUM_FE_H

#include <lithium/x25519.h>

#include <stdint.h>

#ifndef LITH_X25519_WBITS
#define LITH_X25519_WBITS 32
#endif

#if (LITH_X25519_WBITS == 16)

typedef uint16_t limb_t;
typedef uint32_t dlimb_t;
typedef int32_t sdlimb_t;
#define LIMB_MAX 0xFFFFU
#define LIMBS(x) ((limb_t)((x) & LIMB_MAX)), ((limb_t)((x) >> LITH_X25519_WBITS))

#elif (LITH_X25519_WBITS == 32)

typedef uint32_t limb_t;
typedef uint64_t dlimb_t;
typedef int64_t sdlimb_t;
#define LIMB_MAX 0xFFFFFFFFU
#define LIMBS(x) ((limb_t)(x))

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
