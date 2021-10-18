#include <lithium/sign.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char sig[LITH_SIGN_LEN], msg[1024],
        secret_key[LITH_SIGN_SECRET_KEY_LEN];
    tis_make_unknown(msg, sizeof msg);
    tis_make_unknown(secret_key, sizeof secret_key);
    size_t msglen = tis_unsigned_long_interval(0, sizeof msg);
    lith_sign_create(sig, msg, msglen, secret_key);
}
