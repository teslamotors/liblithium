#!/bin/bash

set -xe

name="lithium"
image="$name-builder"

docker build --tag "$image" .

container=$(docker create --tty \
    --volume "$(pwd):/src/$name" \
    --cap-add SYS_PTRACE \
    "$image" \
    "bash" "-c" \
    "git clone $name $name-docker && cd $name-docker && ./build.sh")

if docker start --interactive "$container"; then
    docker cp "$container:/src/$name-docker/dist" "dist"
else
    echo "Build failed, skipping copy out of docker container."
fi

docker rm "$container"
