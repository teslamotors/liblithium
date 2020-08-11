#!/bin/bash

black --check --include "\\.pyi?$|SCons" --exclude "/(\\.git|venv.*|build|.eggs)/" py ffibuilder.py setup.py
