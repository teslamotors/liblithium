/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/random.h>
#include <lithium/x25519.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sc_reduce.h"

static void printx(const unsigned char *x, size_t len)
{
    printf("0x");
    for (int i = (int)len - 1; i >= 0; --i)
    {
        printf("%02hhx", x[i]);
    }
    printf("\n");
}

int main(void)
{
    for (int i = 0; i < 10000; ++i)
    {
        unsigned char s[X25519_LEN * 2];
        lith_random_bytes(s, sizeof s);

        unsigned char li_r[X25519_LEN];
        unsigned char ed_r[X25519_LEN];

        x25519_scalar_reduce(li_r, s);
        sc_reduce(ed_r, s);

        if (memcmp(li_r, ed_r, sizeof li_r) != 0)
        {
            printf("lithium and ed25519 reductions mod L do not match\n");
            printf("s = ");
            printx(s, sizeof s);
            printf("li_r = ");
            printx(li_r, sizeof li_r);
            printf("ed_r = ");
            printx(ed_r, sizeof ed_r);
            return EXIT_FAILURE;
        }
    }

    /* L = 2^252 + 27742317777372353535851937790883648493 */
    static const unsigned char L[X25519_LEN] = {
        0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7,
        0xa2, 0xde, 0xf9, 0xde, 0x14, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
        0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x10,
    };

    unsigned char tv[7][X25519_LEN * 2];
    memset(tv, 0, sizeof tv);

    /* L */
    memcpy(tv[0], L, X25519_LEN);

    /* L + 1 */
    memcpy(tv[1], L, X25519_LEN);
    tv[1][0]++;

    /* L * 256 */
    memcpy(&tv[2][1], L, X25519_LEN);

    /* R = 2^256 */
    tv[3][X25519_LEN] = 1;

    /* R+1 */
    tv[4][X25519_LEN] = 1;
    tv[4][0] = 1;

    /* 0xFF..FF */
    memset(tv[5], 0xFF, X25519_LEN * 2);

    /* tv[6] is 0 */

    for (size_t t = 0; t < sizeof(tv) / sizeof(tv[0]); ++t)
    {
        unsigned char li_r[X25519_LEN];
        unsigned char ed_r[X25519_LEN];

        x25519_scalar_reduce(li_r, tv[t]);
        sc_reduce(ed_r, tv[t]);

        if (memcmp(li_r, ed_r, sizeof li_r) != 0)
        {
            printf("lithium and ed25519 reductions mod L do not match\n");
            printf("s = ");
            printx(tv[t], sizeof tv[t]);
            printf("li_r = ");
            printx(li_r, sizeof li_r);
            printf("ed_r = ");
            printx(ed_r, sizeof ed_r);
            return EXIT_FAILURE;
        }
    }
}
