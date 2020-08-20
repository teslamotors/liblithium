#!/bin/bash

set -xe

[[ ! $# -eq 1 ]] && echo "usage $0 <version>" && exit 1

version="$1"

git tag --annotate --message="lithium release $version" "$version"

git push origin "$version"
