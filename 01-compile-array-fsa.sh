#!/bin/bash -e

cd $(dirname $0)/daram
if [[ ! -d build ]]; then
    mkdir build
fi
cd build
cmake ..
make
ctest

cd ../../experiments
if [[ ! -d build ]]; then
    mkdir build
fi
cd build
cmake ..
make
ctest

