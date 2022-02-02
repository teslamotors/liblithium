/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/random.h>

#include <stdlib.h>

#if defined(__unix__) || defined(__APPLE__)
#define USE_URANDOM 1
#else
#define USE_URANDOM 0
#endif

#ifdef _WIN32
/* Must be included first, so leave a blank line to prevent reordering. */
#include <windows.h>

#include <bcrypt.h>
#include <ntstatus.h>
#include <stdio.h>
#elif USE_URANDOM
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

void lith_random_bytes(unsigned char *buf, size_t len)
{
#ifdef _WIN32
    BCRYPT_ALG_HANDLE h;
    if (BCryptOpenAlgorithmProvider(&h, BCRYPT_RNG_ALGORITHM,
                                    MS_PRIMITIVE_PROVIDER, 0) != STATUS_SUCCESS)
    {
        fprintf(stderr, "failed to open bcrypt algorithm provider\n");
        abort();
    }
    if (BCryptGenRandom(h, buf, len, 0) != STATUS_SUCCESS)
    {
        fprintf(stderr, "failed to generate random data\n");
        BCryptCloseAlgorithmProvider(h, 0);
        abort();
    }
    BCryptCloseAlgorithmProvider(h, 0);
#elif USE_URANDOM
    static const char urandom[] = "/dev/urandom";
    int fd = open(urandom, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open %s: %s\n", urandom, strerror(errno));
        abort();
    }
    if (read(fd, buf, len) != (ssize_t)len)
    {
        fprintf(stderr, "couldn't read %zu bytes from %s: %s\n", len, urandom,
                strerror(errno));
        close(fd);
        abort();
    }
    close(fd);
#else
    /*
     * No supported random implementation available, so abort rather than do
     * something insecure.
     */
    (void)buf;
    (void)len;
    abort();
#endif
}
