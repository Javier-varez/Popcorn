LOCAL_DIR := $(call current-dir)


include $(CLEAR_VARS)

LOCAL_CROSS_COMPILE := arm-none-eabi-
CC := gcc
CXX := g++

LOCAL_NAME := popcorn

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/inc

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -fno-exceptions \
    -fno-rtti

LOCAL_SRC := \
    $(wildcard $(LOCAL_DIR)/src/*.c) \
    $(wildcard $(LOCAL_DIR)/src/*/*.c) \
    $(wildcard $(LOCAL_DIR)/src/*.cpp) \
    $(wildcard $(LOCAL_DIR)/src/*/*.cpp)

LOCAL_EXPORTED_DIRS := \
    $(LOCAL_DIR)/inc
LOCAL_ARFLAGS := -rcs

include $(BUILD_STATIC_LIB)

## Sources to test

TEST_SRC := \
    $(LOCAL_DIR)/src/core/syscalls.cpp \
    $(LOCAL_DIR)/src/core/cortex-m_port.cpp \
    $(LOCAL_DIR)/src/core/kernel.cpp \
    $(LOCAL_DIR)/src/core/lockable.cpp \
    $(LOCAL_DIR)/src/utils/linked_list.c \
    $(LOCAL_DIR)/src/primitives/spinlock.cpp \
    $(LOCAL_DIR)/src/primitives/mutex.cpp \
    $(LOCAL_DIR)/src/primitives/critical_section.cpp

include $(LOCAL_DIR)/test/build.mk

