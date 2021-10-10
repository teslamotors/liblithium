#include <hydrogen.h>

int main(void)
{
    uint8_t h[hydro_hash_BYTES];
    static const uint8_t m[128];
    static const char ctx[hydro_hash_CONTEXTBYTES];
    return hydro_hash_hash(h, sizeof h, m, sizeof m, ctx, NULL);
}
