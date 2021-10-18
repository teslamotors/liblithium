#include <lithium/gimli_hash.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char h[1024], m[1024];
    tis_make_unknown(m, sizeof m);
    size_t hlen = tis_unsigned_long_interval(0, sizeof h);
    size_t mlen = tis_unsigned_long_interval(0, sizeof m);
    gimli_hash(h, hlen, m, mlen);
}
