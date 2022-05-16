#include <lithium/sign.h>

int main(void)
{
    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
        secret_key[LITH_SIGN_SECRET_KEY_LEN];
    lith_sign_keygen(public_key, secret_key);
}
