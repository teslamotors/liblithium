#pragma once

#include <lithium/x25519.h>

#include <stdint.h>

#define WBITS 32
#define NLIMBS (X25519_BITS / WBITS)

typedef uint32_t fe_t[NLIMBS];

void add(fe_t out, const fe_t a, const fe_t b);

void sub(fe_t out, const fe_t a, const fe_t b);

void mul(fe_t out, const fe_t a, const fe_t b);

void mul1(fe_t a, const fe_t b);

void mul_word(fe_t out, const fe_t a, uint32_t b);

void sqr1(fe_t a);

uint32_t canon(fe_t a);

void inv(fe_t out, const fe_t a);
