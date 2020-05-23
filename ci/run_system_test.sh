#!/bin/bash -x

set -e

make -j
make flash

xvfb-run -a ./build/targets/SystemTest
