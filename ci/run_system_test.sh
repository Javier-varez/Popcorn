#!/bin/bash -x

set -e

make flash

./build/targets/SystemTest
