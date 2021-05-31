#ifndef LITHIUM_OPT_H
#define LITHIUM_OPT_H

#include <limits.h>

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
 * If a Gimli state word fits in a machine register, absorb a word at a time.
 */
#ifndef LITH_ABSORB_WORDS
#define LITH_ABSORB_WORDS (UINT_MAX >= UINT32_MAX)
#endif

/*
 * Vector language extensions are supported in clang 4 and gcc 4.7 and above.
 * They are known to work well with SSE+SSE2, ARM NEON, and ARMv8.
 */
#if (defined(__clang__) || (defined(__GNUC__) && defined(__GNUC_MINOR__)))
/* Clang claims a GNUC version of 4.2.1, so check the clang version first. */
#if ((defined(__clang__) && (__clang_major__ >= 4)) ||                         \
     (((__GNUC__ * 100) + __GNUC_MINOR__) >= 407)) &&                          \
    ((defined(__SSE__) && defined(__SSE2__)) ||                                \
     (defined(__ARM_NEON) && defined(__ARM_FEATURE_UNALIGNED)))
#define LITH_VECTORIZE 1
#endif
#endif

#ifndef LITH_VECTORIZE
#define LITH_VECTORIZE 0
#endif

#endif /* LITHIUM_OPT_H */
