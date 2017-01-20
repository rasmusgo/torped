#!/bin/bash
# Install system libs with:
# sudo apt-get install libopenal-dev libalut-dev freeglut3-dev libglew-dev glew-utils liblua5.1-dev libphysfs-dev libtinyxml-dev

set -e # exit on error

( # subshell
    cd "$(dirname '$0')"
    mkdir -p build/
    cd build
    cmake .. && make -j
)
