#include <hydrogen.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr,
                "usage: %s <public-key-file> <message-file> <signature-file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    int pkfd = -1, sigfd = -1, msgfd = -1;
    int exitcode = EXIT_SUCCESS;

    unsigned char public_key[hydro_sign_PUBLICKEYBYTES];
    pkfd = open(argv[1], O_RDONLY);
    if (pkfd < 0)
    {
        perror("could not open public key file");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }
    if (read(pkfd, public_key, sizeof public_key) != sizeof public_key)
    {
        perror("could not read public key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    unsigned char sig[hydro_sign_BYTES];
    sigfd = open(argv[3], O_RDONLY);
    if (sigfd < 0 || read(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not read signature");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    static const char ctx[hydro_sign_CONTEXTBYTES] = {0};
    hydro_sign_state state;
    hydro_sign_init(&state, ctx);

    msgfd = open(argv[2], O_RDONLY);
    if (msgfd < 0)
    {
        perror("could not open message file");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    static unsigned char msg[4096];
    ssize_t nread;

    while ((nread = read(msgfd, msg, sizeof msg)) > 0)
    {
        hydro_sign_update(&state, msg, (size_t)nread);
    }

    if (nread < 0)
    {
        perror("could not read message");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    if (hydro_sign_final_verify(&state, sig, public_key) != 0)
    {
        fprintf(stderr, "could not verify signature\n");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    if ((pkfd >= 0) && (close(pkfd) < 0))
    {
        perror("failed to close public key file");
        exitcode = EXIT_FAILURE;
    }
    if ((sigfd >= 0) && (close(sigfd) < 0))
    {
        perror("failed to close signature file");
        exitcode = EXIT_FAILURE;
    }
    if ((msgfd >= 0) && (close(msgfd) < 0))
    {
        perror("failed to close message file");
        exitcode = EXIT_FAILURE;
    }
    return exitcode;
}
