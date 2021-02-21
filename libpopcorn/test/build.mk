LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)

CC := gcc
CXX := g++

LOCAL_NAME := popcorn_test

LOCAL_CFLAGS := \
    -I$(LOCAL_DIR)/inc \
    -I$(LOCAL_DIR)/../inc/ \
    -DUNITTEST \
    -g3 \
    -Wall \
    -Werror \
    -fprofile-arcs \
    -ftest-coverage

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -std=gnu++17

LOCAL_SRC := \
    $(TEST_SRC) \
    $(wildcard $(LOCAL_DIR)/src/*.c) \
    $(wildcard $(LOCAL_DIR)/src/*.cpp)

LOCAL_LDFLAGS := \
    -lpthread

LOCAL_MULTILIB := 32

include $(BUILD_HOST_TEST)

