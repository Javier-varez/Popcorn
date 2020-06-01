#!/bin/bash -xe

make -j$(nproc) flash

cd SystemTest
xvfb-run -a pytest-3
