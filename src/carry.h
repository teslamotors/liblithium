/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#pragma once

#include <stdint.h>

static uint32_t mac(uint32_t *carry, uint32_t a, uint32_t b, uint32_t c)
{
    uint64_t tmp = (uint64_t)b * c + a + *carry;
    *carry = (uint32_t)(tmp >> 32);
    return (uint32_t)tmp;
}

static uint32_t adc(uint32_t *carry, uint32_t a, uint32_t b)
{
    uint64_t total = (uint64_t)a + b + *carry;
    *carry = (uint32_t)(total >> 32);
    return (uint32_t)total;
}
