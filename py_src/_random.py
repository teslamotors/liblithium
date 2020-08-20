from lithium._lithium import ffi

from secrets import token_bytes


@ffi.def_extern()
def lith_random_bytes(buf, n):
    ffi.memmove(buf, token_bytes(n), n)
