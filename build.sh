#!/bin/bash
set -e # exit on error

( # subshell
    cd "$(dirname '$0')"
    mkdir -p build/
    cd build
    cmake .. && make -j
)
