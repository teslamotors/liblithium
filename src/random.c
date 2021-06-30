/*
 * Part of liblithium, under the Apache License v2.0.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lithium/random.h>

#include <assert.h>

#ifdef _WIN32
#include <winternl.h>
#include <ntstatus.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

void lith_random_bytes(unsigned char *buf, size_t len)
{
#ifdef _WIN32
    BCRYPT_ALG_HANDLE h;
    assert(BCryptOpenAlgorithmProvider(&h, BCRYPT_RNG_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0) == STATUS_SUCCESS);
    assert(BCryptGenRandom(h, buf, len, 0) == STATUS_SUCCESS);
    BCryptCloseAlgorithmProvider(h, 0);
#else
    int fd = open("/dev/urandom", O_RDONLY);
    assert(fd >= 0);
    assert(read(fd, buf, len) == (ssize_t)len);
    close(fd);
#endif
}
