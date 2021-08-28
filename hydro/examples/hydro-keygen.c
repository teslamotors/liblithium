#include <hydrogen.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <key-filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    hydro_sign_keypair kp;
    hydro_sign_keygen(&kp);

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

    skfd = open(skname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (skfd < 0)
    {
        perror("could not open secret key file for writing");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    pkfd = open(pkname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (pkfd < 0)
    {
        perror("could not open public key file for writing");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    if (write(pkfd, kp.pk, sizeof kp.pk) != sizeof kp.pk)
    {
        perror("could not write public key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    if (write(skfd, kp.sk, sizeof kp.sk) != sizeof kp.sk)
    {
        perror("could not write secret key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    if (pkfd >= 0)
    {
        close(pkfd);
    }
    if (skfd >= 0)
    {
        close(skfd);
    }
    return exitcode;
}
