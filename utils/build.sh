#!/bin/bash
set -e

mkdir -p /tmp/build
cd /tmp/build
cmake /tmp/cargo -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CC_COMPILER=$CC -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DWITHOUT_SYSTEMD=1
cmake --build . -- -j4
cmake --build . --target install

cargo_all_tests.py