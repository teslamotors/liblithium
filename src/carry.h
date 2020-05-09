/*
 * Copyright (c) 2015-2016 Cryptography Research, Inc.
 * Released under the MIT License.
 * See LICENSE for license information.
 */

#pragma once

#include <lithium/fe.h>

static limb_t mac(limb_t *carry, limb_t a, limb_t b, limb_t c)
{
    dlimb_t tmp = (dlimb_t)b * c + a + *carry;
    *carry = (limb_t)(tmp >> LITH_X25519_WBITS);
    return (limb_t)tmp;
}

static limb_t adc(limb_t *carry, limb_t a, limb_t b)
{
    dlimb_t total = (dlimb_t)a + b + *carry;
    *carry = (limb_t)(total >> LITH_X25519_WBITS);
    return (limb_t)total;
}
