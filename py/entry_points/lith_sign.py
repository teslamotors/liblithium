# Part of liblithium, under the Apache License v2.0.
# SPDX-License-Identifier: Apache-2.0

from lithium.sign import create

import sys
from pathlib import Path


def main():
    if len(sys.argv) < 4:
        print(
            f"usage: {sys.argv[0]} <secret-key-file> <message-file> <signature-file>",
            file=sys.stderr,
        )
        sys.exit(1)

    sk = Path(sys.argv[1]).read_bytes()
    msg = Path(sys.argv[2]).read_bytes()

    sig = create(msg, sk)

    Path(sys.argv[3]).write_bytes(sig)
