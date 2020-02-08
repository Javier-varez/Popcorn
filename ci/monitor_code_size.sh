#!/bin/bash
TARGET_FILE="build/targets/stm32f1_fw.elf"

make clean
make -j
FINAL_SIZE_OUTPUT_EXACT=$(arm-none-eabi-size $TARGET_FILE)
FINAL_SIZE_OUTPUT=$(arm-none-eabi-size $TARGET_FILE | tail -n 1 | tr -s " " | tr a-z A-Z)
FINAL_TOTAL_SIZE=$(echo $FINAL_SIZE_OUTPUT | cut -d " " -f 4 -)
FINAL_BSS_SIZE=$(echo $FINAL_SIZE_OUTPUT | cut -d " " -f 3 -)
FINAL_DATA_SIZE=$(echo $FINAL_SIZE_OUTPUT | cut -d " " -f 2 -)
FINAL_CODE_SIZE=$(echo $FINAL_SIZE_OUTPUT | cut -d " " -f 1 -)

git checkout dev
git fetch origin dev
git reset --hard FETCH_HEAD

make clean
make -j
ORIGINAL_SIZE_OUTPUT_EXACT=$(arm-none-eabi-size $TARGET_FILE)
ORIGINAL_SIZE_OUTPUT=$(arm-none-eabi-size $TARGET_FILE | tail -n 1 | tr -s " " | tr a-z A-Z)
ORIGINAL_TOTAL_SIZE=$(echo $ORIGINAL_SIZE_OUTPUT | cut -d " " -f 4 -)
ORIGINAL_BSS_SIZE=$(echo $ORIGINAL_SIZE_OUTPUT | cut -d " " -f 3 -)
ORIGINAL_DATA_SIZE=$(echo $ORIGINAL_SIZE_OUTPUT | cut -d " " -f 2 -)
ORIGINAL_CODE_SIZE=$(echo $ORIGINAL_SIZE_OUTPUT | cut -d " " -f 1 -)

let "TOTAL_DELTA=$FINAL_TOTAL_SIZE-$ORIGINAL_TOTAL_SIZE"
let "BSS_DELTA=$FINAL_BSS_SIZE-$ORIGINAL_BSS_SIZE"
let "DATA_DELTA=$FINAL_DATA_SIZE-$ORIGINAL_DATA_SIZE"
let "CODE_DELTA=$FINAL_CODE_SIZE-$ORIGINAL_CODE_SIZE"

echo "Original binary:"
echo "$ORIGINAL_SIZE_OUTPUT_EXACT"
echo ""

echo "Final binary:"
echo "$FINAL_SIZE_OUTPUT_EXACT"
echo ""

echo "Binary size delta for $TARGET_FILE:"
echo -e "\ttext\tdata\tbss\tdec"
echo -e "\t$CODE_DELTA\t$DATA_DELTA\t$BSS_DELTA\t$TOTAL_DELTA\tbytes"
