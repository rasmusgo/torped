#!/bin/bash -e
# This script is only needed on Ubuntu where the packaged SDL2 version is <2.0.5

wget https://libsdl.org/release/SDL2-2.0.5.tar.gz
tar -xzvf SDL2-2.0.5.tar.gz
pushd SDL2-2.0.5
mkdir build
cd build
../configure
make
sudo make install
popd

wget https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.1.tar.gz
tar -xzvf SDL2_image-2.0.1.tar.gz
pushd SDL2_image-2.0.1
./configure --prefix=/usr
make
sudo make install
popd
