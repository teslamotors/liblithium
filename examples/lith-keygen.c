#include <lithium/random.h>
#include <lithium/sign.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#define PLAT_FLAGS O_BINARY
#else
#define PLAT_FLAGS 0
#endif

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

    const char *const skname = argv[1];
    char pkname[255];
    int written = snprintf(pkname, sizeof pkname, "%s.pub", skname);
    if (written < 0)
    {
        perror("snprintf");
        return EXIT_FAILURE;
    }
    if ((size_t)written >= sizeof pkname)
    {
        fprintf(stderr, "key name is too long");
        return EXIT_FAILURE;
    }

    int exitcode = EXIT_SUCCESS;
    int skfd = -1, pkfd = -1;

    skfd = open(skname, O_CREAT | O_WRONLY | O_TRUNC | PLAT_FLAGS, 0600);
    if (skfd < 0)
    {
        perror("could not open secret key file for writing");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    pkfd = open(pkname, O_CREAT | O_WRONLY | O_TRUNC | PLAT_FLAGS, 0600);
    if (pkfd < 0)
    {
        perror("could not open public key file for writing");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    if (write(pkfd, public_key, sizeof public_key) != sizeof public_key)
    {
        perror("could not write public key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    if (write(skfd, secret_key, sizeof secret_key) != sizeof secret_key)
    {
        perror("could not write secret key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    if ((pkfd >= 0) && (close(pkfd) < 0))
    {
        perror("failed to close public key file");
        exitcode = EXIT_FAILURE;
    }
    if ((skfd >= 0) && (close(skfd) < 0))
    {
        perror("failed to close secret key file");
        exitcode = EXIT_FAILURE;
    }
    return exitcode;
}
