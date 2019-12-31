#!/bin/bash -x

set -e

make -j
make flash

./build/targets/SystemTest
