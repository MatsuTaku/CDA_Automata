#!/bin/bash

CURRENT_DIR=$(dirname $0)

cd $CURRENT_DIR/project
mkdir build
cd build
cmake ..
make
