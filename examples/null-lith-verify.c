#include <lithium/sign.h>

int main(void)
{
    static const unsigned char sig[LITH_SIGN_LEN];
    static const unsigned char m[128];
    static const unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN];
    return !lith_sign_verify(sig, m, sizeof m, public_key);
}
