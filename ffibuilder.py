import cffi
import codecs

ffibuilder = cffi.FFI()

c_headers = [
    "lithium/gimli.h",
    "lithium/gimli_state.h",
    "lithium/gimli_hash.h",
    "lithium/sign.h",
]

ffibuilder.set_source(
    "lithium._lithium",
    "\n".join(["#include <%s>" % h for h in c_headers]),
    include_dirs=["include"],
    sources=[
        "src/gimli.c",
        "src/gimli_common.c",
        "src/gimli_hash.c",
        "src/fe.c",
        "src/memzero.c",
        "src/x25519.c",
        "src/sign.c",
    ],
)

cdef = ""

for h in c_headers:
    with codecs.open("include/%s" % h, encoding="utf-8") as f:
        cdef_active = False
        for line in f.read().splitlines():
            if "cffi:begin" in line:
                cdef_active = True
            if "cffi:end" in line:
                cdef_active = False
            if cdef_active:
                cdef += line + "\n"

cdef += """
extern "Python+C" void lith_random_bytes(unsigned char *buf, size_t len);
"""

ffibuilder.cdef(cdef.encode("ascii", "ignore").decode())

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
