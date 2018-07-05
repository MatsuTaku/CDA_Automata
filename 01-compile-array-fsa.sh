#!/bin/bash

CURRENT_DIR=$(dirname $0)

cd $CURRENT_DIR/array-fsa
mkdir build
cd build
cp ../*.hpp ../*.cpp ../CMakeLists.txt .
cmake .
make
#rm -rf *.hpp *.cpp CMake* *.cmake
