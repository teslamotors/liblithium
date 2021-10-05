#ifndef LITHIUM_OPT_H
#define LITHIUM_OPT_H

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>
#include <stdint.h>

/*
 * On little-endian 8-bit byte platforms, accessing the Gimli
 * state words can be done directly with byte accesses.
 */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && (CHAR_BIT == 8)
#define LITH_LITTLE_ENDIAN 1
#endif
#endif

#ifndef LITH_LITTLE_ENDIAN
#define LITH_LITTLE_ENDIAN 0
#endif

/*
 * If a Gimli state word fits in a machine register, sponge operations can
 * happen a word at a time.
 */
#ifndef LITH_SPONGE_WORDS
#define LITH_SPONGE_WORDS (UINT_MAX >= UINT32_MAX)
#endif

/*
 * Vector language extensions are supported in clang 4 and gcc 4.7 and above.
 * They are known to work well with SSE+SSE2, ARM NEON, and ARMv8.
 */
#if (defined(__clang__) || (defined(__GNUC__) && defined(__GNUC_MINOR__)))
/* Clang claims a GNUC version of 4.2.1, so check the clang version first. */
#if ((defined(__clang__) && (__clang_major__ >= 4)) ||                         \
     (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407)) &&                          \
    ((defined(__SSE__) && defined(__SSE2__)) || (defined(__ARM_NEON)))

#define LITH_VECTORIZE 1

/*
 * If vector loads from unaligned addresses are supported, sponge operations can
 * be vectorized. This requires SSE2 on x86 and unaligned accesses on ARM.
 * Vectorized sponge operations also assume little-endian order.
 */
#if (LITH_LITTLE_ENDIAN) &&                                                    \
    (defined(__SSE2__) || defined(__ARM_FEATURE_UNALIGNED))
#define LITH_SPONGE_VECTORS 1
#endif

#endif
#endif

#ifndef LITH_VECTORIZE
#define LITH_VECTORIZE 0
#endif

#ifndef LITH_SPONGE_VECTORS
#define LITH_SPONGE_VECTORS 0
#endif

#if (LITH_SPONGE_VECTORS)
typedef uint32_t block __attribute__((vector_size(16), aligned(1)));
#endif

/*
 * Rotate left by 24 bits can be achieved more efficiently with a byte shuffle,
 * unless there is a vectorized rotate.
 * vpshufb is part of SSSE3, and vprold is part of AVX512VL.
 * Neon doesn't have a vector rotate, but does have shuffles.
 */
#if (defined(__SSSE3__) && !defined(__AVX512VL__)) || defined(__ARM_NEON)
#define LITH_SHUFFLE_ROL24 1
#endif

#ifndef LITH_SHUFFLE_ROL24
#define LITH_SHUFFLE_ROL24 0
#endif

#endif /* LITHIUM_OPT_H */
