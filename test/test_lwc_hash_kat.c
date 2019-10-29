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
// October 28, 2019 to use the liblithium interface to gimli_hash, use stdout
// instead of outputting to a file, and remove unused code.

#include <stdio.h>
#include <string.h>

#include <lithium/gimli_hash.h>

#define MAX_MESSAGE_LENGTH 1024

void init_buffer(unsigned char *buffer, unsigned long long numbytes);

void print_bstr(const char *label, const unsigned char *data,
                unsigned long long length);

int generate_test_vectors(void);

int main(void)
{
    unsigned char msg[MAX_MESSAGE_LENGTH];
    unsigned char digest[GIMLI_HASH_DEFAULT_LEN];
    int count = 1;

    init_buffer(msg, sizeof(msg));

    for (unsigned long long mlen = 0; mlen <= MAX_MESSAGE_LENGTH; mlen++)
    {
        printf("Count = %d\n", count++);
        print_bstr("Msg = ", msg, mlen);
        gimli_hash(digest, GIMLI_HASH_DEFAULT_LEN, msg, mlen);
        print_bstr("MD = ", digest, GIMLI_HASH_DEFAULT_LEN);
        printf("\n");
    }
}

void print_bstr(const char *label, const unsigned char *data,
                unsigned long long length)
{
    printf("%s", label);

    for (unsigned long long i = 0; i < length; i++)
        printf("%02X", data[i]);

    printf("\n");
}

void init_buffer(unsigned char *buffer, unsigned long long numbytes)
{
    for (unsigned long long i = 0; i < numbytes; i++)
        buffer[i] = (unsigned char)i;
}
