#include "hydrogen.h"

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

void hydro_random_buf(void *out, size_t out_len)
{
    int fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    assert(read(fd, out, out_len) == (ssize_t)out_len);
    close(fd);
}
