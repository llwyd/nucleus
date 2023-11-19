#!/bin/sh

if [ ! build/ ]; then
    mkdir build
fi

cd "$(dirname "$0")"
cd build/
make clean

