#!/usr/bin/env bash
set -euo pipefail

# Compile edge case test with sanitizers
CC="${CC:-clang}"
CFLAGS="${CFLAGS:--O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror}"
LDFLAGS="${LDFLAGS:--fsanitize=address,undefined}"

# Include directories
INCLUDE_DIR="-I./include"

echo "Compiling edge case tests..."
$CC $CFLAGS $INCLUDE_DIR test/edge_cases_verify.c -o /tmp/liblithium_edge_tests $LDFLAGS

echo "Running edge case tests..."
/tmp/liblithium_edge_tests

echo "Edge case tests completed successfully!"