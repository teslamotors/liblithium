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
                "usage: %s <public-key-file> <message-file> <signature-file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    unsigned char public_key[LITH_SIGN_PUBLIC_KEY_LEN];
    int pkfd = open(argv[1], O_RDONLY | PLAT_FLAGS);
    if (pkfd < 0 ||
        read(pkfd, public_key, sizeof public_key) != sizeof public_key)
    {
        perror("could not read public key");
        return EXIT_FAILURE;
    }
    close(pkfd);

    unsigned char sig[LITH_SIGN_LEN];
    int sigfd = open(argv[3], O_RDONLY | PLAT_FLAGS);
    if (sigfd < 0 || read(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not read signature");
        return EXIT_FAILURE;
    }
    close(sigfd);

    lith_sign_state state;
    lith_sign_init(&state);

    int msgfd = open(argv[2], O_RDONLY | PLAT_FLAGS);
    if (msgfd < 0)
    {
        perror("could not open message file");
        return EXIT_FAILURE;
    }

    static unsigned char buf[4096];
    ssize_t nread;

    while ((nread = read(msgfd, buf, sizeof buf)) > 0)
    {
        lith_sign_update(&state, buf, (size_t)nread);
    }

    if (nread < 0)
    {
        perror("could not read message");
        return EXIT_FAILURE;
    }
    close(msgfd);

    if (!lith_sign_final_verify(&state, sig, public_key))
    {
        fprintf(stderr, "could not verify signature\n");
        return EXIT_FAILURE;
    }
}
