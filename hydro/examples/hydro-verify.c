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

    unsigned char public_key[hydro_sign_PUBLICKEYBYTES];
    int pkfd = open(argv[1], O_RDONLY);
    if (pkfd < 0 ||
        read(pkfd, public_key, sizeof public_key) != sizeof public_key)
    {
        perror("could not read public key");
        return EXIT_FAILURE;
    }
    close(pkfd);

    unsigned char sig[hydro_sign_BYTES];
    int sigfd = open(argv[3], O_RDONLY);
    if (sigfd < 0 || read(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not read signature");
        return EXIT_FAILURE;
    }
    close(sigfd);

    static const char ctx[hydro_sign_CONTEXTBYTES] = {0};
    hydro_sign_state state;
    hydro_sign_init(&state, ctx);

    int msgfd = open(argv[2], O_RDONLY);
    if (msgfd < 0)
    {
        perror("could not open message file");
        return EXIT_FAILURE;
    }

    static unsigned char buf[4096];
    ssize_t nread;

    while ((nread = read(msgfd, buf, sizeof buf)) > 0)
    {
        hydro_sign_update(&state, buf, (size_t)nread);
    }

    if (nread < 0)
    {
        perror("could not read message");
        return EXIT_FAILURE;
    }
    close(msgfd);

    if (hydro_sign_final_verify(&state, sig, public_key) != 0)
    {
        fprintf(stderr, "could not verify signature\n");
        return EXIT_FAILURE;
    }
}
