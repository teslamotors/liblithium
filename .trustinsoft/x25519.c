#include <lithium/x25519.h>

#include <tis_builtin.h>

int main(void)
{
    unsigned char out[X25519_LEN], scalar[X25519_LEN], point[X25519_LEN];
    tis_make_unknown(scalar, sizeof scalar);
    tis_make_unknown(point, sizeof point);
    x25519(out, scalar, point);
}
