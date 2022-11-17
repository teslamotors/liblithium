#!/usr/bin/env bash

name="liblithium-$(git describe)"

git archive --output="$name.tar.gz" --prefix="$name/" HEAD
