![Lithium](lithium.svg)

# liblithium

liblithium is a lightweight cryptography library that is portable by design. It
requires only standard C99 and does not assume 8-bit addressability, making it
suitable for use on some DSP architectures as well as mainstream architectures.

liblithium is built on the [Gimli permutation](https://gimli.cr.yp.to/) and
X25519 signatures. The Gimli permutation and the Gimli-Hash function are
designed to be high-performance and to have an extremely small footprint.
X25519 signatures are related to the more common ed25519 signatures used by
[NaCl](https://nacl.cr.yp.to/) and others, but use only the x-coordinate of
elliptic curve points, a technique pioneered in the paper ["Fast and compact
elliptic-curve cryptography"](https://www.shiftleft.org/papers/fff/) and
implemented in the [STROBE project](https://sourceforge.net/projects/strobe/).
This technique greatly reduces the code size required for creating and
verifying signatures.
