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
}
