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

    static unsigned char m[4096];
    ssize_t nread;

    while ((nread = read(fd, m, sizeof m)) > 0)
    {
        hydro_hash_update(&state, m, (size_t)nread);
    }

    if (nread == 0)
    {
        unsigned char h[hydro_hash_BYTES];
        hydro_hash_final(&state, h, sizeof h);

        for (size_t i = 0; i < sizeof h; ++i)
        {
            printf("%02hhx", h[i]);
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
                close(fd);
                return EXIT_FAILURE;
            }
            printf("  %s\n", argv[i]);
            close(fd);
        }
    }
}
