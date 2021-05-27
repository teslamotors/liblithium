#include <lithium/gimli_aead.h>

int main(void)
{
    unsigned char c[10] = {0};
    unsigned char t[16] = {0};
    unsigned char m[10] = {0};
    unsigned char ad[10] = {0};
    unsigned char n[GIMLI_AEAD_NONCE_LEN] = {0};
    unsigned char k[GIMLI_AEAD_KEY_LEN] = {0};

    gimli_aead_encrypt(c, t, sizeof t, m, sizeof m, ad, sizeof ad, n, k);
    return gimli_aead_decrypt(m, c, sizeof c, t, sizeof t, ad, sizeof ad, n, k);
}
