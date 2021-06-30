# Part of liblithium, under the Apache License v2.0.
# SPDX-License-Identifier: Apache-2.0

from lithium._lithium import ffi

from secrets import token_bytes


@ffi.def_extern()
def lith_random_bytes(buf, n):
    ffi.memmove(buf, token_bytes(n), n)
