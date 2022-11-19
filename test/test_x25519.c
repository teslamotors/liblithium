/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from STROBE's test_x25519.c, under the MIT license.
 * STROBE is Copyright (c) 2015-2016 Cryptography Research, Inc.
 * SPDX-License-Identifier: MIT
 */

#include <lithium/x25519.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void randomize(unsigned char foo[X25519_LEN])
{
    static unsigned int seed = 0x12345678;
    for (int i = 0; i < X25519_LEN; i++)
    {
        seed += seed * seed | 5;
        foo[i] = (unsigned char)(seed >> 24);
    }
}

static void printx(const unsigned char x[X25519_LEN])
{
    for (int i = 0; i < X25519_LEN; ++i)
    {
        printf("%02hhx", x[i]);
    }
    printf("\n");
}

int main(void)
{
    unsigned char secret1[X25519_LEN], public1[X25519_LEN], secret2[X25519_LEN],
        public2[X25519_LEN], shared1[X25519_LEN], shared2[X25519_LEN];

    for (int i = 0; i < 10; i++)
    {
        randomize(secret1);
        x25519_base(public1, secret1);

        randomize(secret2);
        x25519_base(public2, secret2);

        x25519(shared1, secret1, public2);
        x25519(shared2, secret2, public1);

        if (memcmp(shared1, shared2, sizeof(shared1)))
        {
            printf("FAIL shared %d\n", i);
            return EXIT_FAILURE;
        }
    }

    unsigned char eph_secret[X25519_LEN], eph_public[X25519_LEN],
        challenge[X25519_LEN], response[X25519_LEN];
    for (int i = 0; i < 10; i++)
    {
        randomize(secret1);
        x25519_base_uniform(public1, secret1);
        randomize(eph_secret);
        x25519_base_uniform(eph_public, eph_secret);
        randomize(challenge);
        x25519_sign(response, challenge, eph_secret, secret1);
        if (!x25519_verify(response, challenge, eph_public, public1))
        {
            printf("FAIL sign %d\n", i);
            return EXIT_FAILURE;
        }

        challenge[4] ^= 1;
        if (x25519_verify(response, challenge, eph_public, public1))
        {
            printf("FAIL unsign %d\n", i);
            return EXIT_FAILURE;
        }
    }

    unsigned char base[X25519_LEN] = {9};
    unsigned char key[X25519_LEN] = {9};
    unsigned char *b = base, *k = key, *tmp;

    for (int i = 0; i < 10; i++)
    {
        unsigned char ck[X25519_LEN];
        memcpy(ck, k, X25519_LEN);
        ck[0] &= 0xF8U;
        ck[31] &= 0x7FU;
        ck[31] |= 0x40U;
        x25519(b, ck, b);
        tmp = b;
        b = k;
        k = tmp;
    }
    static const unsigned char expected[X25519_LEN] = {
        0x7BU, 0x96U, 0xF2U, 0x92U, 0xA1U, 0x15U, 0xE8U, 0x3EU,
        0x78U, 0x56U, 0x01U, 0x05U, 0x86U, 0x9CU, 0x00U, 0xB0U,
        0xF4U, 0xFBU, 0x2CU, 0xD1U, 0x0EU, 0x38U, 0x5BU, 0x8CU,
        0x64U, 0xBFU, 0x04U, 0x7BU, 0x0CU, 0x0EU, 0x11U, 0x13U,
    };
    if (memcmp(k, expected, X25519_LEN) != 0)
    {
        printf("FAIL iterated x25519\n");
        return EXIT_FAILURE;
    }

    /*
     * X25519 Test Vectors from RFC7748
     */
    static struct
    {
        unsigned char sc[X25519_LEN];
        unsigned char u[X25519_LEN];
        unsigned char exp[X25519_LEN];
    } tv[] = {
        {
            .sc =
                {
                    0xA5, 0x46, 0xE3, 0x6B, 0xF0, 0x52, 0x7C, 0x9D,
                    0x3B, 0x16, 0x15, 0x4B, 0x82, 0x46, 0x5E, 0xDD,
                    0x62, 0x14, 0x4C, 0x0A, 0xC1, 0xFC, 0x5A, 0x18,
                    0x50, 0x6A, 0x22, 0x44, 0xBA, 0x44, 0x9A, 0xC4,
                },
            .u =
                {
                    0xE6, 0xDB, 0x68, 0x67, 0x58, 0x30, 0x30, 0xDB,
                    0x35, 0x94, 0xC1, 0xA4, 0x24, 0xB1, 0x5F, 0x7C,
                    0x72, 0x66, 0x24, 0xEC, 0x26, 0xB3, 0x35, 0x3B,
                    0x10, 0xA9, 0x03, 0xA6, 0xD0, 0xAB, 0x1C, 0x4C,
                },
            .exp =
                {
                    0xC3, 0xDA, 0x55, 0x37, 0x9D, 0xE9, 0xC6, 0x90,
                    0x8E, 0x94, 0xEA, 0x4D, 0xF2, 0x8D, 0x08, 0x4F,
                    0x32, 0xEC, 0xCF, 0x03, 0x49, 0x1C, 0x71, 0xF7,
                    0x54, 0xB4, 0x07, 0x55, 0x77, 0xA2, 0x85, 0x52,
                },
        },
        {
            .sc =
                {
                    0x4B, 0x66, 0xE9, 0xD4, 0xD1, 0xB4, 0x67, 0x3C,
                    0x5A, 0xD2, 0x26, 0x91, 0x95, 0x7D, 0x6A, 0xF5,
                    0xC1, 0x1B, 0x64, 0x21, 0xE0, 0xEA, 0x01, 0xD4,
                    0x2C, 0xA4, 0x16, 0x9E, 0x79, 0x18, 0xBA, 0x0D,
                },
            .u =
                {
                    0xE5, 0x21, 0x0F, 0x12, 0x78, 0x68, 0x11, 0xD3,
                    0xF4, 0xB7, 0x95, 0x9D, 0x05, 0x38, 0xAE, 0x2C,
                    0x31, 0xDB, 0xE7, 0x10, 0x6F, 0xC0, 0x3C, 0x3E,
                    0xFC, 0x4C, 0xD5, 0x49, 0xC7, 0x15, 0xA4, 0x13,
                    /*
                     * See RFC errata: https://www.rfc-editor.org/errata/eid5568
                     * Original RFC has last octet as 0x93 instead of 0x13.
                     */
                },
            .exp =
                {
                    0x95, 0xCB, 0xDE, 0x94, 0x76, 0xE8, 0x90, 0x7D,
                    0x7A, 0xAD, 0xE4, 0x5C, 0xB4, 0xB8, 0x73, 0xF8,
                    0x8B, 0x59, 0x5A, 0x68, 0x79, 0x9F, 0xA1, 0x52,
                    0xE6, 0xF8, 0xF7, 0x64, 0x7A, 0xAC, 0x79, 0x57,
                },
        },
    };

    for (size_t t = 0; t < sizeof(tv) / sizeof(tv[0]); ++t)
    {
        unsigned char exp[X25519_LEN];
        x25519(exp, tv[t].sc, tv[t].u);
        if (memcmp(exp, tv[t].exp, X25519_LEN) != 0)
        {
            printf("expected: ");
            printx(tv[t].exp);
            printf("received: ");
            printx(exp);
            return EXIT_FAILURE;
        }
    }
}
