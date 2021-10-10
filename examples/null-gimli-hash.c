#include <lithium/gimli_hash.h>

int main(void)
{
    unsigned char h[GIMLI_HASH_DEFAULT_LEN];
    static const unsigned char m[128];
    gimli_hash(h, sizeof h, m, sizeof m);
    return h[0] != h[31];
}
