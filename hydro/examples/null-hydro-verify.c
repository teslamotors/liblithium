#include <hydrogen.h>

int main(void)
{
    static const uint8_t csig[hydro_sign_BYTES];
    static const char ctx[hydro_sign_CONTEXTBYTES];
    static const uint8_t pk[hydro_sign_PUBLICKEYBYTES];
    return hydro_sign_verify(csig, NULL, 0, ctx, pk);
}
