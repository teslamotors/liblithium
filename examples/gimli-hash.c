#include <lithium/gimli_hash.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#define PLAT_FLAGS O_BINARY
#else
#define PLAT_FLAGS 0
#endif

static ssize_t hash_fd(int fd)
{
    gimli_hash_state state;
    gimli_hash_init(&state);

    static unsigned char buf[4096];
    ssize_t nread;

    while ((nread = read(fd, buf, sizeof buf)) > 0)
    {
        gimli_hash_update(&state, buf, (size_t)nread);
    }

    if (nread == 0)
    {
        unsigned char output[GIMLI_HASH_DEFAULT_LEN];
        gimli_hash_final(&state, output, sizeof output);

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
            int fd = open(argv[i], O_RDONLY | PLAT_FLAGS);
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
