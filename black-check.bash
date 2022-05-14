#!/usr/bin/env bash

mapfile -t pyfiles < <(git ls-files | grep -E "\\.pyi?$|SCons")

black --check --diff "${pyfiles[@]}"
