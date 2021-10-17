#include <lithium/random.h>
#include <lithium/sign.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
        secret_key[LITH_SIGN_SECRET_KEY_LEN], sig[LITH_SIGN_LEN], msg[1024];
    size_t msglen = tis_unsigned_long_interval(0, sizeof msg);
    lith_sign_keygen(public_key, secret_key);
    lith_random_bytes(msg, sizeof msg);
    lith_random_bytes(sig, sizeof sig);
    return !lith_sign_verify(sig, msg, msglen, public_key);
}
