/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/gimli.h>

#include <stdio.h>

int main()
{
    uint32_t x[GIMLI_WORDS];
    unsigned int i;

    for (i = 0; i < GIMLI_WORDS; ++i)
        x[i] = i * i * i + i * 0x9E3779B9;

    gimli(x);

    for (i = 0; i < GIMLI_WORDS; ++i)
    {
        printf("%08x ", x[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    return 0;
}
