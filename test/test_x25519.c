/*
 * Copyright (c) 2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
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
            fprintf(stderr, "FAIL shared %d\n", i);
            return EXIT_FAILURE;
        }
    }

    unsigned char eph_secret[X25519_LEN], eph_public[X25519_LEN],
        challenge[X25519_LEN], response[X25519_LEN];
    for (int i = 0; i < 10; i++)
    {
        randomize(secret1);
        x25519_base(public1, secret1);
        randomize(eph_secret);
        x25519_base(eph_public, eph_secret);
        randomize(challenge);
        x25519_sign(response, challenge, eph_secret, secret1);
        if (!x25519_verify(response, challenge, eph_public, public1))
        {
            fprintf(stderr, "FAIL sign %d\n", i);
            return EXIT_FAILURE;
        }

        challenge[4] ^= 1;
        if (x25519_verify(response, challenge, eph_public, public1))
        {
            fprintf(stderr, "FAIL unsign %d\n", i);
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
        fprintf(stderr, "FAIL iterated x25519\n");
        return EXIT_FAILURE;
    };

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
                    0xa5, 0x46, 0xe3, 0x6b, 0xf0, 0x52, 0x7c, 0x9d,
                    0x3b, 0x16, 0x15, 0x4b, 0x82, 0x46, 0x5e, 0xdd,
                    0x62, 0x14, 0x4c, 0x0a, 0xc1, 0xfc, 0x5a, 0x18,
                    0x50, 0x6a, 0x22, 0x44, 0xba, 0x44, 0x9a, 0xc4,
                },
            .u =
                {
                    0xe6, 0xdb, 0x68, 0x67, 0x58, 0x30, 0x30, 0xdb,
                    0x35, 0x94, 0xc1, 0xa4, 0x24, 0xb1, 0x5f, 0x7c,
                    0x72, 0x66, 0x24, 0xec, 0x26, 0xb3, 0x35, 0x3b,
                    0x10, 0xa9, 0x03, 0xa6, 0xd0, 0xab, 0x1c, 0x4c,
                },
            .exp =
                {
                    0xc3, 0xda, 0x55, 0x37, 0x9d, 0xe9, 0xc6, 0x90,
                    0x8e, 0x94, 0xea, 0x4d, 0xf2, 0x8d, 0x08, 0x4f,
                    0x32, 0xec, 0xcf, 0x03, 0x49, 0x1c, 0x71, 0xf7,
                    0x54, 0xb4, 0x07, 0x55, 0x77, 0xa2, 0x85, 0x52,
                },
        },
        {
            .sc =
                {
                    0x4b, 0x66, 0xe9, 0xd4, 0xd1, 0xb4, 0x67, 0x3c,
                    0x5a, 0xd2, 0x26, 0x91, 0x95, 0x7d, 0x6a, 0xf5,
                    0xc1, 0x1b, 0x64, 0x21, 0xe0, 0xea, 0x01, 0xd4,
                    0x2c, 0xa4, 0x16, 0x9e, 0x79, 0x18, 0xba, 0x0d,
                },
            .u =
                {
                    0xe5, 0x21, 0x0f, 0x12, 0x78, 0x68, 0x11, 0xd3,
                    0xf4, 0xb7, 0x95, 0x9d, 0x05, 0x38, 0xae, 0x2c,
                    0x31, 0xdb, 0xe7, 0x10, 0x6f, 0xc0, 0x3c, 0x3e,
                    0xfc, 0x4c, 0xd5, 0x49, 0xc7, 0x15, 0xa4, 0x13,
                    /*
                     * See RFC errata: https://www.rfc-editor.org/errata/eid5568
                     * Original RFC has last octet as 0x93 instead of 0x13.
                     */
                },
            .exp =
                {
                    0x95, 0xcb, 0xde, 0x94, 0x76, 0xe8, 0x90, 0x7d,
                    0x7a, 0xad, 0xe4, 0x5c, 0xb4, 0xb8, 0x73, 0xf8,
                    0x8b, 0x59, 0x5a, 0x68, 0x79, 0x9f, 0xa1, 0x52,
                    0xe6, 0xf8, 0xf7, 0x64, 0x7a, 0xac, 0x79, 0x57,
                },
        },
    };

    for (size_t t = 0; t < sizeof(tv) / sizeof(tv[0]); ++t)
    {
        unsigned char sc[X25519_LEN];
        memcpy(sc, tv[t].sc, X25519_LEN);
        x25519_clamp(sc);
        unsigned char exp[X25519_LEN];
        x25519(exp, sc, tv[t].u);
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
