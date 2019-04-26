#include <lithium/gimli_hash.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* normal print */
static void print_output(uint8_t *x)
{
    int i;
    for (i = 0; i < 32; ++i)
        printf("%02x", x[i]);
    printf("\n");
}

static void print_hex(uint8_t *x, size_t l)
{
    for (size_t i = 0; i < l; ++i)
    {
        printf("%02x", x[i]);
        if (i % 4 == 3)
            printf(" ");
    }
    printf("\n");
}

static void print_test(unsigned char *string, uint8_t *output)
{
    printf("input: %s\n", string);
    gimli_hash(output, 32, string, strlen((char *)string));
    printf("-\n");
    printf("input: ");
    print_hex(string, strlen((char *)string));
    print_output(output);
    printf("----------------------\n");
}

/* print in stderr to generate the testVectors.tex ! */
static void print_output_err(uint8_t *x)
{
    int i;
    fprintf(stderr, "\t{\\tt ");
    for (i = 0; i < 32; ++i)
        fprintf(stderr, "%02x", x[i]);
    fprintf(stderr, "}\\\\\n");
}

static void print_hex_err(uint8_t *x, size_t l)
{
    fprintf(stderr, "\t{\\tt ");
    for (size_t i = 0; i < l; ++i)
    {
        fprintf(stderr, "%02x", x[i]);
        if (i % 4 == 3)
            fprintf(stderr, " ");
    }
    fprintf(stderr, "}\\\\\n");
}

static void print_tex(unsigned char *string, uint8_t *output)
{
    size_t l = strlen((char *)string);
    fprintf(stderr, "\t\\noindent\n");
    if (l > 0)
    {
        fprintf(stderr, "\t{\\bf input:} \"%s\"\\\\\n", string);
        fprintf(stderr, "\t{\\bf input (bytes):}\\\\\n");
        print_hex_err(string, strlen((char *)string));
    }
    else
    {
        fprintf(stderr, "\t{\\bf input:} \"\" (empty string)\\\\\n");
        fprintf(stderr, "\t{\\bf input (bytes):} {\\it(0 bytes)}\\\\\n");
    }
    fprintf(stderr, "\t{\\bf output:}\\\\\n");
    gimli_hash(output, 32, string, strlen((char *)string));
    print_output_err(output);
    fprintf(stderr, "\t\\medskip\n\n");
}

int main()
{
    unsigned char string1[] =
        "There's plenty for the both of us, may the best Dwarf win.";
    unsigned char string2[] =
        "If anyone was to ask for my opinion, which I note they're "
        "not, I'd say we were taking the long way around.";
    unsigned char string3[] = "Speak words we can all understand!";
    unsigned char string4[] =
        "It's true you don't see many Dwarf-women. And in fact, they are so "
        "alike in voice and appearance, that they are often mistaken for "
        "Dwarf-men.  And this in turn has given rise to the belief that there "
        "are no Dwarf-women, and that Dwarves just spring out of holes in the "
        "ground! Which is, of course, ridiculous.";
    unsigned char string5[] = "";

    uint8_t output[32];

    printf("----------------------\n");

    print_test(string1, output);
    print_test(string2, output);
    print_test(string3, output);
    print_test(string4, output);
    print_test(string5, output);

    fprintf(stderr, "\\section{Test Vectors for \\gimli-Hash}\n");
    print_tex(string1, output);
    print_tex(string2, output);
    // print_tex(string3,output);
    print_tex(string4, output);
    print_tex(string5, output);

    return 0;
}
