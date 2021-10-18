#include <lithium/sign.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#define PLAT_FLAGS O_BINARY
#else
#define PLAT_FLAGS 0
#endif

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr,
                "usage: %s <secret-key-file> <message-file> <signature-file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    int skfd = -1, sigfd = -1, msgfd = -1;
    int exitcode = EXIT_SUCCESS;

    unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN];
    skfd = open(argv[1], O_RDONLY | PLAT_FLAGS);
    if (skfd < 0)
    {
        perror("could not open the secret key file");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }
    if (read(skfd, secret_key, sizeof secret_key) != sizeof secret_key)
    {
        perror("could not read secret key");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    unsigned char sig[LITH_SIGN_LEN];
    sigfd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC | PLAT_FLAGS, 0600);
    if (sigfd < 0)
    {
        perror("could not create signature file");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    lith_sign_state state;
    lith_sign_init(&state);

    msgfd = open(argv[2], O_RDONLY | PLAT_FLAGS);
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
        lith_sign_update(&state, msg, (size_t)nread);
    }

    if (nread < 0)
    {
        perror("could not read message");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

    lith_sign_final_create(&state, sig, secret_key);

    if (write(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not write signature");
        exitcode = EXIT_FAILURE;
        goto cleanup;
    }

cleanup:
    if ((skfd >= 0) && (close(skfd) < 0))
    {
        perror("failed to close secret key file");
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
