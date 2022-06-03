#!/bin/bash

set -xe

name="lithium"
image="$name-builder"

docker build --tag "$image" .

workdir="/mnt/liblithium"

docker run \
  --rm \
  --tty \
  --interactive \
  --volume "$(pwd):$workdir" \
  --workdir "$workdir" \
  --cap-add SYS_PTRACE \
  "$image"
