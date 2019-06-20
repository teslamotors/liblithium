#include <lithium/sign.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr,
                "usage: %s <message-file> <secret-key-file> <signature-file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    lith_sign_state state;
    lith_sign_init(&state);

    int msgfd = open(argv[1], O_RDONLY);
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

    unsigned char secret_key[LITH_SIGN_SECRET_KEY_LEN];
    int skfd = open(argv[2], O_RDONLY);
    if (skfd < 0)
    {
        perror("could not open secret key file");
        return EXIT_FAILURE;
    }

    if (read(skfd, secret_key, sizeof secret_key) != sizeof secret_key)
    {
        perror("could not read secret key");
        return EXIT_FAILURE;
    }

    unsigned char sig[LITH_SIGN_LEN];
    lith_sign_final_create(&state, sig, secret_key);

    int sigfd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (sigfd < 0)
    {
        perror("could not open signature file for writing");
        return EXIT_FAILURE;
    }

    if (write(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not write signature");
        return EXIT_FAILURE;
    }
}
