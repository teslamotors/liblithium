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
        foo[i] = seed >> 24;
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
    for (i = 0; i < X25519_LEN; i++)
        printf("%02x", k[i]);
    printf("\n");
    return 0;
}
