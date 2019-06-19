#include <lithium/gimli_hash.h>

#include <stdio.h>
#include <unistd.h>

int main(void)
{
    unsigned char output[32];

    gimli_hash_state state;
    gimli_hash_init(&state);

    static unsigned char buf[4096];
    ssize_t nread;

    while ((nread = read(STDIN_FILENO, buf, sizeof buf)) > 0)
    {
        gimli_hash_update(&state, buf, (size_t)nread);
    }

    gimli_hash_final(&state, output, sizeof output);

    for (size_t i = 0; i < sizeof output; ++i)
    {
        printf("%02hhx", output[i]);
    }
    printf("\n");
}
