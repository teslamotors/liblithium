#include <lithium/gimli.h>

typedef uint32_t row_t __attribute__((vector_size(16)));

#if defined(__clang_version__)
#define SHUFFLE(x, ...) __builtin_shufflevector((x), (x), __VA_ARGS__)
#elif defined(__GNUC__)
#define SHUFFLE(x, ...) __builtin_shuffle((x), (typeof(x)){__VA_ARGS__})
#else
#error "no suitable shuffle intrinsic"
#endif

static inline row_t rol(row_t x, int n)
{
    if (n == 0)
    {
        return x;
    }
    return x << n | x >> (32 - n);
}

static inline row_t rol24(row_t x)
{
    typedef uint8_t row8_t __attribute__((vector_size(16)));
    row8_t xb = (row8_t)x;
    xb = SHUFFLE(xb, 1, 2, 3, 0, 5, 6, 7, 4, 9, 10, 11, 8, 13, 14, 15, 12);
    return (row_t)xb;
}

static inline row_t load(uint32_t *p)
{
    return (row_t){p[0], p[1], p[2], p[3]};
}

static inline void store(uint32_t *p, row_t x)
{
    p[0] = x[0];
    p[1] = x[1];
    p[2] = x[2];
    p[3] = x[3];
}

static inline row_t small_swap(row_t x)
{
    return SHUFFLE(x, 1, 0, 3, 2);
}

static inline row_t big_swap(row_t x)
{
    return SHUFFLE(x, 2, 3, 0, 1);
}

static inline void sp(row_t *x, row_t *y, row_t *z)
{
    *x = rol24(*x);
    *y = rol(*y, 9);
    const row_t newz = *x ^ (*z << 1) ^ ((*y & *z) << 2);
    const row_t newy = *y ^ *x ^ ((*x | *z) << 1);
    *x = *z ^ *y ^ ((*x & *y) << 3);
    *y = newy;
    *z = newz;
}

static inline row_t coeff(int round)
{
    return (row_t){UINT32_C(0x9e377900) | (uint32_t)round, 0, 0, 0};
}

void gimli(uint32_t *state)
{
    row_t x = load(&state[0]);
    row_t y = load(&state[4]);
    row_t z = load(&state[8]);

    for (int round = 24; round > 0; round -= 4)
    {
        sp(&x, &y, &z);
        x = small_swap(x) ^ coeff(round);
        sp(&x, &y, &z);
        sp(&x, &y, &z);
        x = big_swap(x);
        sp(&x, &y, &z);
    }

    store(&state[0], x);
    store(&state[4], y);
    store(&state[8], z);
}
