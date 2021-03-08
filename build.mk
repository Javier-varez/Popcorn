LOCAL_DIR := $(call current-dir)

GLOBAL_CFLAGS := \
    -Os \
    -g3 \
    -Wall \
    -Werror \
    -Wno-gnu-string-literal-operator-template \
    -ffunction-sections \
    -fdata-sections

TARGET_CFLAGS := \
    -mthumb \
    -mcpu=cortex-m3 \
    -DSTM32F103xB \
    $(GLOBAL_CFLAGS)

GLOBAL_CXXFLAGS := \
    -std=gnu++17 \
    -fno-exceptions \
    -fno-rtti

TARGET_CXXFLAGS := \
    $(GLOBAL_CXXFLAGS)

include $(call all-makefiles-under, $(LOCAL_DIR))

