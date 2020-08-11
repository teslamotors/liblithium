#!/bin/bash

set -xe

virtualenv -p python3.7 venv_black
source venv_black/bin/activate
pip install black==19.10b0
./black-check.sh
deactivate

scons --jobs "$(nproc)"

function pybuild {
    version="python$1"
    venv="venv$1"
    virtualenv -p "$version" "$venv"
    source "$venv/bin/activate"
    pip install . pytest
    pytest --verbose --color=yes test
    python setup.py sdist bdist_wheel
    deactivate
    rm -rf "test/__pycache__"
}

pybuild 3.6
pybuild 3.7
