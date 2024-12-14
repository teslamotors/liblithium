# Part of liblithium, under the Apache License v2.0.
# SPDX-License-Identifier: Apache-2.0

from lithium.sign import keygen

import sys
from pathlib import Path


def main():
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <key-filename>", file=sys.stderr)
        sys.exit(1)

    pk, sk = keygen()

    Path(sys.argv[1]).write_bytes(sk)
    Path(sys.argv[1] + ".pub").write_bytes(pk)
