[![TrustInSoft CI](https://ci.trust-in-soft.com/projects/teslamotors/liblithium.svg?branch=main)](https://ci.trust-in-soft.com/projects/teslamotors/liblithium)

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
verifying signatures. liblithium's X25519 implementation is derived from
STROBE.

# Compiling

While you can embed liblithium in many environments, the library comes with a
SConstruct file for building using scons by default.

You can also use the `docker.sh` script that will conveniently build
liblithium along with examples for multiple target architectures.

# What you can use liblithium for

liblithium is particularly well-suited for constrained environments and
low-power microcontrollers due to its very small footprint and limited
processing requirements. This makes liblithium a great candidate for
implementing signed firmware updates on embedded electronics that have no
secure boot functionality.

## Basics of using liblithium for signed updates

Before anything else, you should ensure that all debug ports (e.g., JTAG) on
your target MCU are disabled, since those can be used to circumvent
software-only signature verification.

Signature verification should ideally be implemented in the bootloader, either
at boot time, or only at firmware update time if boot speed is critical.
Note that for update-time-only checks, this mechanism will only be effective
for MCUs where the entire application is stored in internal flash and protected
from read/write via a debugger (see statement on JTAG lock above).

The bootloader must contain the public key that will be used for signature
verification. The corresponding secret key must be kept confidential and will
be used for signing firmware update binaries.

In order for the signature verification process to be effective, the entire
firmware binary should be signed (not only the header or a subset of the
firmware).

Since signature verification can be done continuously during data reception by
the update process, it makes sense to append the signature at the end of the
firmware binary, since the signature is required at that point for final
verification.

# Examples

## Generating a signature

You can refer to [`examples/lith-sign.c`](examples/lith-sign.c) for an example
of how to sign a binary blob with a secret key.

Three calls only are required to implement this:

- `lith_sign_init(&state);` : initializes the liblithium library state (state
  is a `lith_sign_state`)
- `lith_sign_update(&state, msg, len);` : updates the liblithium
  state for each data block that is being read
- `lith_sign_final_create(&state, sig, secret_key);` : is called once all the
  data is received, and generates the signature using the secret key.

## Verifying a signature

You can refer to [`examples/lith-verify.c`](examples/lith-verify.c) for an
example of how to verify the signature of a binary blob against a public key.

Three calls only are required to implement this:

- `lith_sign_init(&state);` : initializes the liblithium state (state is
   a `lith_sign_state`)
- `lith_sign_update(&state, msg, len);` : updates the liblithium
  state for each data block that is being read (for instance when
  reading a file, or receiving data over a serial bus)
- `lith_sign_final_verify(&state, sig, public_key);` : is called once all the
  data and the signature are received, and verifies the signature against the
  public key.
