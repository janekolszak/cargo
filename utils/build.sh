#!/bin/bash
set -e

# Build
mkdir -p /tmp/build
cd /tmp/build
cmake /tmp/cargo -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CC_COMPILER=$CC -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug -DWITHOUT_SYSTEMD=1
cmake --build . -- -j4
cmake --build . --target install


# Run tests
rm -rf /tmp/ut-*
cargo_all_tests.py
# cargo_launch_test.py --gdb cargo-unit-tests

# sleep 10000
# cargo_launch_test.py --gdb cargo-unit-tests
# gdb /tmp/build/tests/unit_tests/cargo-unit-tests