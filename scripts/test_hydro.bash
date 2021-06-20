#!/bin/bash

set -xe

scons --no-sanitize --jobs $(nproc) dist/hydro

testdir="dist/test/hydrotest"
PATH="$(pwd)/dist/hydro:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

hydro-keygen testkey
head -c 10000000 /dev/urandom > input.bin
cat input.bin > /dev/null
time hydro-sign testkey input.bin input.sig
hydro-verify testkey.pub input.bin input.sig

[ -d libhydrogen ] || git clone https://github.com/jedisct1/libhydrogen.git

CCFLAGS="-g -O3 -flto -march=native"

clang $CCFLAGS -c -o hydrogen.o -Ilibhydrogen libhydrogen/hydrogen.c
clang $CCFLAGS -o libhydrogen-sign -Ilibhydrogen hydrogen.o ../../../hydro/examples/hydro-sign.c
clang $CCFLAGS -o libhydrogen-verify -Ilibhydrogen hydrogen.o ../../../hydro/examples/hydro-verify.c
clang $CCFLAGS -o libhydrogen-hash -Ilibhydrogen hydrogen.o ../../../hydro/examples/hydro-hash.c

# check that libhydrogen can verify signatures from hydro
./libhydrogen-verify testkey.pub input.bin input.sig

# sign with libhydrogen and check that hydro can verify
time ./libhydrogen-sign testkey input.bin input_libhydrogen.sig
hydro-verify testkey.pub input.bin input_libhydrogen.sig
