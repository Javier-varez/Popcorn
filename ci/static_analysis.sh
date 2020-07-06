#!/bin/bash -xe

make clean

ARM_TOOLCHAIN=$(which arm-none-eabi-gcc)/../../

mkdir -p build/static_analysis

pvs-studio-analyzer trace \
    -o build/static_analysis/strace \
    -- \
    make build/targets/stm32f1_fw.elf -j$(nproc)

pvs-studio-analyzer analyze \
    -f build/static_analysis/strace \
    -o build/static_analysis/project.log \
    -j$(nproc) \
    -a 28 \
    -e $ARM_TOOLCHAIN \
    -e $PWD/build/lib/libstm32cubef1/exports
plog-converter \
    -t fullhtml \
    -dV1042 \
    -a 'GA:1,2,3;OP:1,2,3;CS:1,2,3' \
    -o build/static_analysis/HTML \
    build/static_analysis/project.log \
    | tee build/static_analysis.log

grep build/static_analysis.log -e "Filtered messages: 0"
