#!/bin/bash

cd $(dirname $0)/daram
mkdir build
cd build
cmake ..
make
ctest

cd ../../experiments
mkdir build
cd build
cmake ..
make
ctest

