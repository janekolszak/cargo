#!/bin/bash
set -e

rm -rf /tmp/build
mkdir -p /tmp/build
cd /tmp/build
cmake /tmp/cargo -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CC_COMPILER=$CC -DCMAKE_BUILD_TYPE=Release
make -j2
make install
