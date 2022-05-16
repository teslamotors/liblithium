#include <lithium/gimli_aead.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char m[1024], c[1024], t[32], ad[1024], n[GIMLI_AEAD_NONCE_LEN],
        k[GIMLI_AEAD_KEY_LEN];
    tis_make_unknown(c, sizeof c);
    tis_make_unknown(t, sizeof t);
    tis_make_unknown(ad, sizeof ad);
    tis_make_unknown(n, sizeof n);
    tis_make_unknown(k, sizeof k);
    /*
     * When using tis_unsigned_long_interval, TIS doesn't keep a single
     * (arbitrary) value of len between decrypt_update and the masking loop
     * for authentication failure, so it warns on potentially uninitialized
     * access in the latter. This could be suppressed by initializing m up
     * front, but this would hide all possible uninitialized accesses to m. We
     * still want to check that all of m is set by decrypt_update, for some
     * length, we just can't check this easily for a range of lengths at once
     * without false positives. So just use sizeof c for len here.
     */
    size_t len = sizeof c;
    size_t tlen = tis_unsigned_long_interval(0, sizeof t);
    size_t adlen = tis_unsigned_long_interval(0, sizeof ad);
    return !gimli_aead_decrypt(m, c, len, t, tlen, ad, adlen, n, k);
}
