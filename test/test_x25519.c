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
    unsigned i;
    static unsigned int seed = 0x12345678;

    for (i = 0; i < X25519_LEN; i++)
    {
        seed += seed * seed | 5;
        foo[i] = (unsigned char)(seed >> 24);
    }
}

int main(void)
{

    int i;

    unsigned char secret1[X25519_LEN], public1[X25519_LEN], secret2[X25519_LEN],
        public2[X25519_LEN], shared1[X25519_LEN], shared2[X25519_LEN];

    for (i = 0; i < 1000; i++)
    {
        randomize(secret1);
        x25519_base(public1, secret1, i % 2);

        randomize(secret2);
        x25519_base(public2, secret2, i % 2);

        x25519(shared1, secret1, public2, i % 2);
        x25519(shared2, secret2, public1, i % 2);

        if (memcmp(shared1, shared2, sizeof(shared1)))
        {
            fprintf(stderr, "FAIL shared %d\n", i);
            return EXIT_FAILURE;
        }
    }

    unsigned char eph_secret[X25519_LEN], eph_public[X25519_LEN],
        challenge[X25519_LEN], response[X25519_LEN];
    for (i = 0; i < 1000; i++)
    {
        randomize(secret1);
        x25519_base(public1, secret1, 0);
        randomize(eph_secret);
        x25519_base(eph_public, eph_secret, 0);
        randomize(challenge);
        x25519_sign_p2(response, challenge, eph_secret, secret1);
        if (0 != x25519_verify_p2(response, challenge, eph_public, public1))
        {
            fprintf(stderr, "FAIL sign %d\n", i);
            return EXIT_FAILURE;
        }

        challenge[4] ^= 1;
        if (0 == x25519_verify_p2(response, challenge, eph_public, public1))
        {
            fprintf(stderr, "FAIL unsign %d\n", i);
            return EXIT_FAILURE;
        }
    }

    unsigned char base[X25519_LEN] = {9};
    unsigned char key[X25519_LEN] = {9};
    unsigned char *b = base, *k = key, *tmp;

    for (i = 0; i < 1000; i++)
    {
        x25519(b, k, b, 1);
        tmp = b;
        b = k;
        k = tmp;
    }
    static const unsigned char expected[X25519_LEN] = {
        0x68U, 0x4CU, 0xF5U, 0x9BU, 0xA8U, 0x33U, 0x09U, 0x55U,
        0x28U, 0x00U, 0xEFU, 0x56U, 0x6FU, 0x2FU, 0x4DU, 0x3CU,
        0x1CU, 0x38U, 0x87U, 0xC4U, 0x93U, 0x60U, 0xE3U, 0x87U,
        0x5FU, 0x2EU, 0xB9U, 0x4DU, 0x99U, 0x53U, 0x2CU, 0x51U,
    };
    if (memcmp(k, expected, X25519_LEN) != 0)
    {
        fprintf(stderr, "FAIL iterated x25519\n");
        return EXIT_FAILURE;
    };
}
