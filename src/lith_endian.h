#ifndef LITHIUM_LITH_ENDIAN_H
#define LITHIUM_LITH_ENDIAN_H

#include <limits.h>

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && (CHAR_BIT == 8)
#define LITH_LITTLE_ENDIAN 1
#endif
#endif

#ifndef LITH_LITTLE_ENDIAN
#define LITH_LITTLE_ENDIAN 0
#endif

#endif /* LITHIUM_LITH_ENDIAN_H */
