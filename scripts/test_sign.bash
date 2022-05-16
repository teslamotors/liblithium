#!/bin/bash

set -xe

scons --jobs $(nproc) build/lith-keygen build/lith-sign build/lith-verify

testdir="build/test/signtest"
PATH="$(pwd)/build:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

lith-keygen testkey
head -c 100000000 /dev/urandom > input.bin
time lith-sign testkey input.bin input.sig
lith-verify testkey.pub input.bin input.sig

popd
