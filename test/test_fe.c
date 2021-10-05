/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/fe.h>

#include <assert.h>
#include <string.h>

int main(void)
{
    fe m, z, a;
    memset(m, 0xFF, sizeof m);
    memset(z, 0, sizeof z);
    sub(a, z, m);
    add(a, a, m);
    canon(a);
    assert(memcmp(a, z, sizeof a) == 0);
}
