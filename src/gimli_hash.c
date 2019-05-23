#include <lithium/gimli_hash.h>

#include <lithium/gimli.h>

#include <string.h>

void gimli_hash_init(gimli_hash_state *state)
{
    memset(state, 0, sizeof(*state));
}

void gimli_hash_update(gimli_hash_state *state, const unsigned char *input,
                       size_t len)
{
    size_t offset = state->offset;
    for (size_t i = 0; i < len; ++i)
    {
        gimli_xor8(state->state, offset, input[i]);
        ++offset;
        if (offset == GIMLI_RATE)
        {
            gimli(state->state);
            offset = 0;
        }
    }
    state->offset = offset;
}

void gimli_hash_final(gimli_hash_state *state, unsigned char *output,
                      size_t len)
{
    // Apply padding.
    gimli_xor8(state->state, state->offset, 0x1F);
    gimli_xor8(state->state, GIMLI_RATE - 1, 0x80);

    // Switch to the squeezing phase.
    for (size_t i = 0; i < len; ++i)
    {
        if (i % GIMLI_RATE == 0)
        {
            gimli(state->state);
        }
        output[i] = gimli_read8(state->state, i % GIMLI_RATE);
    }
}

void gimli_hash(unsigned char *output, size_t output_len,
                const unsigned char *input, size_t input_len)
{
    gimli_hash_state state;
    gimli_hash_init(&state);
    gimli_hash_update(&state, input, input_len);
    gimli_hash_final(&state, output, output_len);
}
