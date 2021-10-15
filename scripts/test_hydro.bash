#!/bin/bash

set -xe

scons --no-sanitize --jobs "$(nproc)" dist/hydro

testdir="dist/test/hydrotest"
PATH="$(pwd)/dist/hydro:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

hydro-keygen testkey
head -c 10000000 /dev/urandom > input.bin
cat input.bin > /dev/null
time hydro-sign testkey input.bin input.sig
hydro-verify testkey.pub input.bin input.sig

if [ -d libhydrogen ]; then
    pushd libhydrogen
    git checkout master
    git pull
    popd
else
    git clone https://github.com/jedisct1/libhydrogen.git
fi

if [[ "$(uname -ps)" == "Darwin arm" ]]; then
    arch_flag="-mcpu=apple-a14"
else
    arch_flag="-march=native"
fi

CC="clang"
CCFLAGS="-g -O3 -flto $arch_flag -Ilibhydrogen"

$CC $CCFLAGS -c -o hydrogen.o libhydrogen/hydrogen.c
$CC $CCFLAGS -o libhydrogen-sign hydrogen.o ../../../hydro/examples/hydro-sign.c
$CC $CCFLAGS -o libhydrogen-verify hydrogen.o ../../../hydro/examples/hydro-verify.c
$CC $CCFLAGS -o libhydrogen-hash hydrogen.o ../../../hydro/examples/hydro-hash.c

# check that libhydrogen can verify signatures from hydro
./libhydrogen-verify testkey.pub input.bin input.sig

# sign with libhydrogen and check that hydro can verify
time ./libhydrogen-sign testkey input.bin input_libhydrogen.sig
hydro-verify testkey.pub input.bin input_libhydrogen.sig

h1=$(./libhydrogen-hash input.bin)
h2=$(hydro-hash input.bin)
if [ ! "$h1" = "$h2" ]; then
    echo "Hashes don't match."
fi

CC="arm-none-eabi-gcc"
CCFLAGS="-Wl,--gc-sections -ffunction-sections -fdata-sections -specs=nosys.specs -specs=nano.specs -g -Os -mcpu=cortex-m4 -flto -fdump-rtl-expand -Ilibhydrogen -D__unix__"

$CC $CCFLAGS -c -o arm_hydrogen.o libhydrogen/hydrogen.c
$CC $CCFLAGS -Wl,--entry=hydro_sign_verify -o libhydrogen_sign_verify arm_hydrogen.o
$CC $CCFLAGS -Wl,--entry=hydro_hash_hash -o libhydrogen_hash_hash arm_hydrogen.o

scons -C ../../.. --no-sanitize --jobs "$(nproc)" dist/arm/entrypoints

arm-none-eabi-size \
  libhydrogen_sign_verify \
  ../../arm/entrypoints/hydro_sign_verify \
  ../../arm/entrypoints/lith_sign_verify \
  libhydrogen_hash_hash \
  ../../arm/entrypoints/hydro_hash_hash \
  ../../arm/entrypoints/gimli_hash
