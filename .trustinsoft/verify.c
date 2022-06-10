#include <lithium/sign.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char sig[LITH_SIGN_LEN], msg[32],
        public_key[LITH_SIGN_PUBLIC_KEY_LEN];
    tis_make_unknown(sig, sizeof sig);
    tis_make_unknown(msg, sizeof msg);
    tis_make_unknown(public_key, sizeof public_key);
    size_t msglen = tis_unsigned_long_interval(0, sizeof msg);
    return !lith_sign_verify(sig, msg, msglen, public_key);
}
