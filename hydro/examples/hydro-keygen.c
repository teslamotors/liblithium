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

    char *keyname = argv[1];
    char *pubkeyname = malloc(strlen(keyname) + 5);
    if (!pubkeyname)
    {
        perror("malloc");
        return EXIT_FAILURE;
    }
    strcpy(pubkeyname, keyname);
    strcat(pubkeyname, ".pub");

    int exitcode = EXIT_SUCCESS;
    int pkfd = -1, skfd = -1;

    pkfd = open(pubkeyname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    free(pubkeyname);
    if (pkfd < 0)
    {
        perror("could not open public key file for writing");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    skfd = open(keyname, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (skfd < 0)
    {
        perror("could not open secret key file for writing");
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
