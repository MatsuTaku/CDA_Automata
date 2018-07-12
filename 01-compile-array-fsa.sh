#!/bin/bash

CURRENT_DIR=$(dirname $0)

cd $CURRENT_DIR/array-fsa
mkdir build
cd build
cmake ..
make
