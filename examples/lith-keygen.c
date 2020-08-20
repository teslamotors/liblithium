#include <lithium/random.h>
#include <lithium/sign.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <winternl.h>
#include <ntstatus.h>
#include <bcrypt.h>
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <key-filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN],
        secret_key[LITH_SIGN_SECRET_KEY_LEN];
    lith_sign_keygen(public_key, secret_key);

    char *keyname = argv[1];
    char *pubkeyname = malloc(strlen(keyname) + 5);
    strcpy(pubkeyname, keyname);
    strcat(pubkeyname, ".pub");

    int pkfd = open(pubkeyname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    free(pubkeyname);
    if (pkfd < 0)
    {
        perror("could not open public key file for writing");
        return EXIT_FAILURE;
    }

    int skfd = open(keyname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (skfd < 0)
    {
        perror("could not open secret key file for writing");
        return EXIT_FAILURE;
    }

    if (write(pkfd, public_key, sizeof public_key) != sizeof public_key)
    {
        perror("could not write public key");
        return EXIT_FAILURE;
    }

    if (write(skfd, secret_key, sizeof secret_key) != sizeof secret_key)
    {
        perror("could not write secret key");
        return EXIT_FAILURE;
    }

    close(pkfd);
    close(skfd);
}
