/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/random.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winternl.h>

#include <bcrypt.h>
#include <ntstatus.h>
#else
#include <fcntl.h>
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
#else
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
#endif
}
