//
// NIST-developed software is provided by NIST as a public service.
// You may use, copy and distribute copies of the software in any medium,
// provided that you keep intact this entire notice. You may improve,
// modify and create derivative works of the software or any portion of
// the software, and you may copy and distribute such modifications or
// works. Modified works should carry a notice stating that you changed
// the software and should note the date and nature of any such change.
// Please explicitly acknowledge the National Institute of Standards and
// Technology as the source of the software.
//
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO
// WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION
// OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA
// ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE
// SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE
// USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
// CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
//
// You are solely responsible for determining the appropriateness of using and
// distributing the software and you assume all risks associated with its use,
// including but not limited to the risks and costs of program errors,
// compliance with applicable laws, damage to or loss of data, programs or
// equipment, and the unavailability or interruption of operation. This software
// is not intended to be used in any situation where a failure could cause risk
// of injury or damage to property. The software developed by NIST employees is
// not subject to copyright protection within the United States.
//

// This file was incorporated into the liblithium test suite and modified on
// November 13, 2019 to use the liblithium interface to gimli_aead, use stdout
// instead of outputting to a file, and remove unused code.

// This file was modified on November 20, 2019 to add a test for whether
// the decrypt operation will fail if any of the ciphertext, tag, associated
// data, nonce, or key is modified, and that the message output is zeroed out
// in this case.

#include <stdio.h>
#include <string.h>

#include <lithium/gimli_aead.h>

#define KAT_SUCCESS 0
#define KAT_CRYPTO_FAILURE -4

#define MAX_MESSAGE_LENGTH 32
#define MAX_ASSOCIATED_DATA_LENGTH 32

void init_buffer(unsigned char *buffer, size_t numbytes);

void print_bstr(const char *label, const unsigned char *data, size_t length);

int generate_test_vectors(void);

int main(void)
{
    int ret = generate_test_vectors();

    if (ret != KAT_SUCCESS)
    {
        fprintf(stderr, "test vector generation failed with code %d\n", ret);
    }

    return ret;
}

int generate_test_vectors(void)
{
    unsigned char key[GIMLI_AEAD_KEY_LEN];
    unsigned char nonce[GIMLI_AEAD_NONCE_LEN];
    unsigned char msg[MAX_MESSAGE_LENGTH];
    unsigned char msg2[MAX_MESSAGE_LENGTH];
    unsigned char ad[MAX_ASSOCIATED_DATA_LENGTH];
    unsigned char ct[MAX_MESSAGE_LENGTH + GIMLI_AEAD_TAG_DEFAULT_LEN];
    int count = 1;
    int func_ret;

    init_buffer(key, sizeof(key));
    init_buffer(nonce, sizeof(nonce));
    init_buffer(msg, sizeof(msg));
    init_buffer(ad, sizeof(ad));

    for (size_t mlen = 0; mlen <= MAX_MESSAGE_LENGTH; mlen++)
    {

        for (size_t adlen = 0; adlen <= MAX_ASSOCIATED_DATA_LENGTH; adlen++)
        {
            printf("Count = %d\n", count++);
            print_bstr("Key = ", key, GIMLI_AEAD_KEY_LEN);
            print_bstr("Nonce = ", nonce, GIMLI_AEAD_NONCE_LEN);
            print_bstr("PT = ", msg, mlen);
            print_bstr("AD = ", ad, adlen);
            gimli_aead_encrypt(ct, &ct[mlen], GIMLI_AEAD_TAG_DEFAULT_LEN, msg,
                               mlen, ad, adlen, nonce, key);
            print_bstr("CT = ", ct, mlen + GIMLI_AEAD_TAG_DEFAULT_LEN);
            printf("\n");

            if ((func_ret = !gimli_aead_decrypt(msg2, ct, mlen, &ct[mlen],
                                                GIMLI_AEAD_TAG_DEFAULT_LEN, ad,
                                                adlen, nonce, key)))
            {
                printf("gimli_aead_decrypt returned <%d>\n", func_ret);
                return KAT_CRYPTO_FAILURE;
            }

            if (memcmp(msg, msg2, mlen))
            {
                printf("gimli_aead_decrypt did not recover the plaintext\n");
                return KAT_CRYPTO_FAILURE;
            }

            unsigned char *const p[] = {
                mlen > 0 ? ct : NULL,
                &ct[mlen],
                adlen > 0 ? ad : NULL,
                nonce,
                key,
            };
            for (size_t i = 0; i < sizeof(p) / sizeof(p[0]); ++i)
            {
                if (p[i] != NULL)
                {
                    *p[i] ^= 0xffU;
                    if (gimli_aead_decrypt(msg2, ct, mlen, &ct[mlen],
                                           GIMLI_AEAD_TAG_DEFAULT_LEN, ad,
                                           adlen, nonce, key))
                    {
                        printf("gimli_aead_decrypt succeeded on an invalid "
                               "input\n");
                        return KAT_CRYPTO_FAILURE;
                    }
                    for (size_t j = 0; j < mlen; ++j)
                    {
                        if (msg2[j] != 0)
                        {
                            printf("gimli_aead_decrypt did not clear the "
                                   "plaintext on authentication failure\n");
                            return KAT_CRYPTO_FAILURE;
                        }
                    }
                    *p[i] ^= 0xffU;
                }
            }
        }
    }

    return KAT_SUCCESS;
}

void print_bstr(const char *label, const unsigned char *data, size_t length)
{
    printf("%s", label);

    for (size_t i = 0; i < length; i++)
        printf("%02X", data[i]);

    printf("\n");
}

void init_buffer(unsigned char *buffer, size_t numbytes)
{
    for (size_t i = 0; i < numbytes; i++)
        buffer[i] = (unsigned char)i;
}
