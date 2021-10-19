#include <lithium/gimli_aead.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char c[1024], t[32], m[1024], ad[1024], n[GIMLI_AEAD_NONCE_LEN],
        k[GIMLI_AEAD_KEY_LEN];
    tis_make_unknown(m, sizeof m);
    tis_make_unknown(ad, sizeof ad);
    tis_make_unknown(n, sizeof n);
    tis_make_unknown(k, sizeof k);
    size_t tlen = tis_unsigned_long_interval(0, sizeof t);
    size_t len = tis_unsigned_long_interval(0, sizeof m);
    size_t adlen = tis_unsigned_long_interval(0, sizeof ad);
    gimli_aead_encrypt(c, t, tlen, m, len, ad, adlen, n, k);
}
