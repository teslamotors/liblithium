/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from musl libc's explicit_bzero.c, under the MIT license.
 * musl libc is Copyright © 2005-2020 Rich Felker, et al.
 * SPDX-License-Identifier: MIT
 */

#include "memzero.h"

#if defined(_WIN32)
#include <windows.h>

#include <wincrypt.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <string.h>
#endif

void lith_memzero(void *p, size_t len)
{
#if defined(_WIN32)
    SecureZeroMemory(p, len);
#elif defined(__GNUC__) || defined(__clang__)
    /*
     * Derived from musl libc's implementation of explicit_bzero.
     *
     * See these emails from Rich Felker for an explanation.
     * https://www.openwall.com/lists/musl/2015/01/29/3
     * https://www.openwall.com/lists/musl/2018/06/28/9
     * https://www.openwall.com/lists/musl/2018/06/28/11
     *
     * "As long as there's a barrier, LTO is no problem. The asm is a black
     * box that's required to see the results of memset, since the address of
     * the object reaches the asm, and the only way to ensure that such a black
     * box sees the writes is for them to actually be performed."
     */
    __asm__ __volatile__("" : : "r"(memset(p, 0, len)) : "memory");
#else
    /*
     * Fall back to writing through a volatile pointer if nothing else is
     * available.
     *
     * See this post about why this approach is not guaranteed to work:
     * https://www.daemonology.net/blog/2014-09-04-how-to-zero-a-buffer.html
     *
     * "The C standard states that accesses to volatile objects are part of the
     * unalterable observable behaviour — but it says nothing about accesses
     * via lvalue expressions with volatile types. Consequently a sufficiently
     * intelligent compiler can still optimize the buffer-zeroing away in this
     * case — it just has to prove that the object being accessed was not
     * originally defined as being volatile.
     *
     * Some people will try this with secure_memzero in a separate C file. This
     * will trick yet more compilers, but no guarantees — with link-time
     * optimization the compiler may still discover your treachery."
     *
     * We'll treat it as good enough if we're on a platform that doesn't
     * support the inline assembly statement required to prevent optimization.
     * Such compilers are hopefully not devious enough to optimize this away,
     * or don't have LTO.
     */
    volatile unsigned char *pv = p;
    size_t i;
    for (i = 0; i < len; ++i)
    {
        pv[i] = 0;
    }
#endif
}
