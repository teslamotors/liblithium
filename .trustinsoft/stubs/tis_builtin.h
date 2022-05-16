#ifndef STUBS_TIS_BUILTIN_H
#define STUBS_TIS_BUILTIN_H

#include <string.h>

/*
 * Trivial versions of tis_builtin.h functions to build the examples against
 * outside of the TIS environment.
 */

static inline int tis_make_unknown(void *p, unsigned long len)
{
    memset(p, 0, len);
    return 0;
}

static inline unsigned long tis_unsigned_long_interval(unsigned long min_,
                                                       unsigned long max_)
{
    (void)max_;
    return min_;
}

#endif // STUBS_TIS_BUILTIN_H
