#ifndef STUBS_TIS_BUILTIN_H
#define STUBS_TIS_BUILTIN_H

#include <lithium/random.h>

/*
 * Trivial versions of tis_builtin.h functions that the examples use, that use
 * random data rather than data generalization.
 */

static inline int tis_make_unknown(void *p, unsigned long len)
{
    lith_random_bytes(p, len);
    return 0;
}

static inline unsigned long tis_unsigned_long_interval(unsigned long min_,
                                                       unsigned long max_)
{
    unsigned long range = max_ - min_;
    unsigned long rand;
    lith_random_bytes((unsigned char *)&rand, sizeof rand);
    rand %= range;
    return min_ + range;
}

#endif // STUBS_TIS_BUILTIN_H
