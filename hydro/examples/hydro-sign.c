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
                "usage: %s <secret-key-file> <message-file> <signature-file>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    unsigned char secret_key[hydro_sign_SECRETKEYBYTES];
    int skfd = open(argv[1], O_RDONLY);
    if (skfd < 0 ||
        read(skfd, secret_key, sizeof secret_key) != sizeof secret_key)
    {
        perror("could not read secret key");
        return EXIT_FAILURE;
    }
    close(skfd);

    unsigned char sig[hydro_sign_BYTES];
    int sigfd = open(argv[3], O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (sigfd < 0)
    {
        perror("could not create signature file");
        return EXIT_FAILURE;
    }

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

    hydro_sign_final_create(&state, sig, secret_key);

    if (write(sigfd, sig, sizeof sig) != sizeof sig)
    {
        perror("could not write signature");
        return EXIT_FAILURE;
    }
    close(sigfd);
}
