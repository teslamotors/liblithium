#!/bin/bash

set -xe

scons --jobs $(nproc)

testdir="dist/test/signtest"
PATH="$(pwd)/dist:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

lith-keygen testkey
head -c 100000000 /dev/urandom > input.bin
time lith-sign testkey input.bin input.sig
lith-verify testkey.pub input.bin input.sig
