#include <hydrogen.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static ssize_t hash_fd(int fd)
{
    hydro_hash_state state;
    static const char ctx[hydro_hash_CONTEXTBYTES] = {0};
    hydro_hash_init(&state, ctx, NULL);

    static unsigned char buf[4096];
    ssize_t nread;

    while ((nread = read(fd, buf, sizeof buf)) > 0)
    {
        hydro_hash_update(&state, buf, (size_t)nread);
    }

    if (nread == 0)
    {
        unsigned char output[hydro_hash_BYTES];
        hydro_hash_final(&state, output, sizeof output);

        for (size_t i = 0; i < sizeof output; ++i)
        {
            printf("%02hhx", output[i]);
        }
    }

    return nread;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        if (hash_fd(STDIN_FILENO) < 0)
        {
            perror("read");
            return EXIT_FAILURE;
        }
        printf("  -\n");
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0)
            {
                perror("open");
                return EXIT_FAILURE;
            }
            if (hash_fd(fd) < 0)
            {
                perror("read");
                return EXIT_FAILURE;
            }
            printf("  %s\n", argv[i]);
            close(fd);
        }
    }
}
