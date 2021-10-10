#include <hydrogen.h>

int main(void)
{
    static const uint8_t csig[hydro_sign_BYTES];
    static const uint8_t m[128];
    static const char ctx[hydro_sign_CONTEXTBYTES];
    static const uint8_t pk[hydro_sign_PUBLICKEYBYTES];
    return hydro_sign_verify(csig, m, sizeof m, ctx, pk);
}
