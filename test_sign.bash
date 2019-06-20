#!/bin/bash

set -xe

scons

testdir="dist/test/signtest"
PATH="$(pwd)/dist:$PATH"

mkdir -p "$testdir"
pushd "$testdir"

simple-keygen testkey
head -c 1000000 /dev/urandom > input.bin
simple-sign input.bin testkey input.sig
simple-verify input.bin input.sig testkey.pub
