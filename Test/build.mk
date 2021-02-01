LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)

CC := gcc
CXX := g++

LOCAL_NAME := TestOS

LOCAL_CFLAGS := \
    -I$(LOCAL_DIR)/../ \
    -I$(LOCAL_DIR)/../OS \
    -DUNITTEST \
    -g3 \
    -Wall \
    -Werror \
    -fprofile-arcs \
    -ftest-coverage

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS)

TEST_SRC := \
    $(LOCAL_DIR)/../OS/Src/core/syscalls.cpp \
    $(LOCAL_DIR)/../OS/Src/core/cortex-m_port.cpp \
    $(LOCAL_DIR)/../OS/Src/core/kernel.cpp \
    $(LOCAL_DIR)/../OS/Src/core/lockable.cpp \
    $(LOCAL_DIR)/../OS/Src/utils/linked_list.c \
    $(LOCAL_DIR)/../OS/Src/primitives/spinlock.cpp \
    $(LOCAL_DIR)/../OS/Src/primitives/mutex.cpp \
    $(LOCAL_DIR)/../OS/Src/primitives/critical_section.cpp

LOCAL_SRC := \
    $(TEST_SRC) \
    $(wildcard $(LOCAL_DIR)/Src/*.c) \
    $(wildcard $(LOCAL_DIR)/Src/*.cpp)

LOCAL_LDFLAGS := \
    -lpthread \
    -lstdc++

LOCAL_MULTILIB := 32

include $(BUILD_HOST_TEST)

