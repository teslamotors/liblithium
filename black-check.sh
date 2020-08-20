#!/bin/bash

black --check --include "\\.pyi?$|SCons" --exclude "/(\\.git|venv.*|build|.eggs)/" py_src ffibuilder.py setup.py
