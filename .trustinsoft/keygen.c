#include <lithium/sign.h>

#include <tis_builtin.h>

void lith_random_bytes(unsigned char *buf, size_t len)
{
    tis_make_unknown(buf, len);
}

int main(void)
{
    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
        secret_key[LITH_SIGN_SECRET_KEY_LEN];
    lith_sign_keygen(public_key, secret_key);
}
