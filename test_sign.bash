#!/bin/bash

set -xe

scons

testdir="dist/test/signtest"
PATH="$(pwd)/dist:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

simple-keygen testkey
head -c 1000000 /dev/urandom > input.bin
simple-sign testkey input.bin input.sig
simple-verify testkey.pub input.bin input.sig
