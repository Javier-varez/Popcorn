LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)

LOCAL_CROSS_COMPILE := arm-none-eabi-
CC := gcc
CXX := g++

LOCAL_NAME := stm32f1_fw.elf

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/. \
    -I$(LOCAL_DIR)/Inc

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -fno-exceptions \
    -fno-rtti

LOCAL_LDFLAGS := \
    -Wl,--gc-sections \
    --specs=nano.specs \
    --specs=nosys.specs \
    -lstdc++

LOCAL_LINKER_FILE := \
    $(LOCAL_DIR)/STM32F103X8_FLASH.ld

LOCAL_SRC := \
    $(wildcard $(LOCAL_DIR)/Src/*.c) \
    $(wildcard $(LOCAL_DIR)/Src/*.cpp)

LOCAL_STATIC_LIBS := \
    libstm32cubef1 \
    libos

include $(BUILD_BINARY)

