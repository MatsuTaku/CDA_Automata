#!/bin/bash

CURRENT_DIR=$(dirname $0)

cd $CURRENT_DIR/src
mkdir build
cd build
cmake ..
make
