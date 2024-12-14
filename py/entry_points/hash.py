# Part of liblithium, under the Apache License v2.0.
# SPDX-License-Identifier: Apache-2.0

from lithium.gimli_hash import gimli_hash

import sys
from pathlib import Path


def main():
    if len(sys.argv) < 2:
        print(f"{gimli_hash(bytes(sys.stdin.read(), encoding='utf8')).hex()}  -")
    else:
        for filename in sys.argv[1:]:
            print(f"{gimli_hash(Path(filename).read_bytes()).hex()}  {filename}")
