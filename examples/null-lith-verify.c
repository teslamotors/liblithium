#include <lithium/sign.h>

#include <stdlib.h>

int main(void)
{
    lith_sign_state state;
    lith_sign_init(&state);

    unsigned char *volatile p = NULL;
    volatile size_t n = 1;

    while (n != 0)
    {
        lith_sign_update(&state, p, n);
    }

    static unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN] = {0};
    static unsigned char sig[LITH_SIGN_LEN] = {0};

    if (!lith_sign_final_verify(&state, sig, public_key))
    {
        return EXIT_FAILURE;
    }
}
