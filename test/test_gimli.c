#include <stdio.h>

extern void gimli(unsigned int *);

int main()
{
    unsigned int x[12];
    unsigned int i;

    for (i = 0; i < 12; ++i)
        x[i] = i * i * i + i * 0x9E3779B9;

    gimli(x);

    for (i = 0; i < 12; ++i)
    {
        printf("%08x ", x[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    return 0;
}
