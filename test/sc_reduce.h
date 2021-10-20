/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Derived from the SUPERCOP ed25519 ref10 implementation in the public domain.
 * https://ed25519.cr.yp.to/software.html
 */

#ifndef SC_REDUCE_H
#define SC_REDUCE_H

void sc_reduce(unsigned char *r, unsigned char *s);

#endif // SC_REDUCE_H
