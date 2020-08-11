#!/bin/bash

set -x

name="lithium"
image="$name-builder"

docker build --tag "$image" .

cidfile="$(mktemp -u cidfile.XXXXXX)"

docker run --interactive --tty \
    --volume "$(pwd):/src/$name" \
    --cidfile "$cidfile" \
    --cap-add SYS_PTRACE \
    "$image" \
    "bash" "-c" \
    "git clone $name $name-docker && cd $name-docker && ./build.sh"

container="$(cat "$cidfile")"
docker cp "$container:/src/$name-docker/dist" "dist"
docker rm "$container"
rm -f "$cidfile"
