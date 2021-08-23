#include <lithium/random.h>
#include <lithium/sign.h>

#include <assert.h>

/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from libhydrogen's tests.c, under the ISC license.
 * libhydrogen is Copyright (c) 2017-2021 Frank Denis <j at pureftpd dot org>
 * SPDX-License-Identifier: ISC
 */

static void test_sign(void)
{
    unsigned char msg[500];
    unsigned char sig[LITH_SIGN_LEN];
    lith_sign_state st;
    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
        secret_key[LITH_SIGN_SECRET_KEY_LEN];
    lith_random_bytes(msg, sizeof msg);
    lith_sign_keygen(public_key, secret_key);
    lith_sign_create(sig, msg, sizeof msg, secret_key);
    assert(lith_sign_verify(sig, msg, sizeof msg, public_key));
    sig[0]++;
    assert(!lith_sign_verify(sig, msg, sizeof msg, public_key));
    sig[0]--;
    sig[LITH_SIGN_LEN - 1]++;
    assert(!lith_sign_verify(sig, msg, sizeof msg, public_key));
    sig[LITH_SIGN_LEN - 1]--;
    msg[0]++;
    assert(!lith_sign_verify(sig, msg, sizeof msg, public_key));
    msg[0]++;
    lith_sign_create(sig, msg, sizeof msg, secret_key);

    lith_sign_init(&st);
    lith_sign_update(&st, msg, (sizeof msg) / 3);
    lith_sign_update(&st, msg + (sizeof msg) / 3,
                     (sizeof msg) - (sizeof msg) / 3);
    assert(lith_sign_final_verify(&st, sig, public_key));

    lith_sign_init(&st);
    lith_sign_update(&st, msg, (sizeof msg) / 3);
    lith_sign_update(&st, msg + (sizeof msg) / 3,
                     (sizeof msg) - (sizeof msg) / 3);
    lith_sign_final_create(&st, sig, secret_key);

    lith_sign_init(&st);
    lith_sign_update(&st, msg, (sizeof msg) / 3);
    lith_sign_update(&st, msg + (sizeof msg) / 3,
                     (sizeof msg) - (sizeof msg) / 3);
    assert(lith_sign_final_verify(&st, sig, public_key));

    lith_sign_init(&st);
    lith_sign_update(&st, msg, (sizeof msg) / 3);
    lith_sign_update(&st, msg + (sizeof msg) / 3,
                     (sizeof msg) - (sizeof msg) / 3);
    sig[0]++;
    assert(!lith_sign_final_verify(&st, sig, public_key));

    lith_sign_create(sig, msg, 0, secret_key);
    assert(!lith_sign_verify(sig, msg, sizeof msg, public_key));
    assert(lith_sign_verify(sig, msg, 0, public_key));
}

int main(void)
{
    test_sign();
}
