#include <lithium/random.h>

#include <tis_builtin.h>

void lith_random_bytes(unsigned char *buf, size_t len)
{
    tis_make_unknown(buf, len);
}
