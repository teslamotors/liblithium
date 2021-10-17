#!/bin/bash

set -xe

name="lithium"
image="$name-builder"

docker build --tag "$image" .

workdir="/mnt/liblithium"

docker run \
  --platform="linux/amd64" \
  --rm \
  --tty \
  --interactive \
  --volume "$(pwd):$workdir" \
  --user "$(id -u $USER):$(id -g $USER)" \
  --workdir "$workdir" \
  --cap-add SYS_PTRACE \
  "$image" \
  "bash" "-c" 'scons --jobs "$(nproc)" --mingw --powerpc'
