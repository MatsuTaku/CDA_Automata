#!/bin/bash

DIR_ROOT=$(dirname $0)

cd $DIR_ROOT
mkdir build
cd build
cmake ..
make
ctest

