LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)

LOCAL_CROSS_COMPILE := arm-none-eabi-
CC := gcc
CXX := g++

LOCAL_NAME := popcorn_stm32f103

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/inc

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
    $(LOCAL_DIR)/src/main.cpp \
    $(LOCAL_DIR)/src/startup.cpp \
    $(LOCAL_DIR)/src/uart.cpp \
    $(LOCAL_DIR)/src/system_stm32f1xx.c

LOCAL_STATIC_LIBS := \
    libstm32cubef1 \
    libpopcorn

include $(BUILD_BINARY)

