LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)

CC := clang
CXX := clang++

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
    $(GLOBAL_CXXFLAGS)

LOCAL_SRC := \
    $(TEST_SRC) \
    $(LOCAL_DIR)/src/cortex-m_port_test.cpp \
    $(LOCAL_DIR)/src/kernel_test.cpp \
    $(LOCAL_DIR)/src/linked_list_test.cpp \
    $(LOCAL_DIR)/src/mock_assert.cpp \
    $(LOCAL_DIR)/src/mock_mcu.cpp \
    $(LOCAL_DIR)/src/mock_mem_management.cpp \
    $(LOCAL_DIR)/src/mutex_test.cpp \
    $(LOCAL_DIR)/src/spinlock_test.cpp \
    $(LOCAL_DIR)/src/syscall_test.cpp

LOCAL_LDFLAGS := \
    -lpthread

LOCAL_MULTILIB := 32

include $(BUILD_HOST_TEST)
