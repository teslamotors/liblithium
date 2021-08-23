#!/bin/bash

set -xe

name="lithium"
image="$name-builder"
container="$name-container"

docker build --tag "$image" .

if docker run \
  --name "$container" \
  --tty \
  --interactive \
  --volume "$(pwd):/src/$name" \
  --cap-add SYS_PTRACE \
  "$image" \
  "bash" "-c" \
  "git clone $name $name-docker && cd $name-docker && ./build.sh"
then
  docker cp "$container:/src/$name-docker/dist" "dist"
else
  echo "Build failed, skipping copy out of docker container."
fi

docker rm "$container"
