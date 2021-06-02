#!/bin/bash

name="liblithium-$(git describe)"

git archive --output="$name.tar.gz" --prefix="$name/" HEAD
