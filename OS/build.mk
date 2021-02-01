LOCAL_DIR := $(call current-dir)


include $(CLEAR_VARS)

LOCAL_CROSS_COMPILE := arm-none-eabi-
CC := gcc
CXX := g++

LOCAL_NAME := os

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/.

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -fno-exceptions \
    -fno-rtti

LOCAL_SRC := \
    $(wildcard $(LOCAL_DIR)/Src/*.c) \
    $(wildcard $(LOCAL_DIR)/Src/*/*.c) \
    $(wildcard $(LOCAL_DIR)/Src/*.cpp) \
    $(wildcard $(LOCAL_DIR)/Src/*/*.cpp)

LOCAL_EXPORTED_DIRS := \
    $(LOCAL_DIR)
LOCAL_ARFLAGS := -rcs

include $(BUILD_STATIC_LIB)

