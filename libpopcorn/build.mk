LOCAL_DIR := $(call current-dir)


include $(CLEAR_VARS)

LOCAL_NAME := popcorn

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/inc

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    $(TARGET_CXXFLAGS)

LOCAL_SRC := \
    $(LOCAL_DIR)/src/core/cortex-m_port.cpp \
    $(LOCAL_DIR)/src/core/cortex-m_port_asm.cpp \
    $(LOCAL_DIR)/src/primitives/critical_section.cpp \
    $(LOCAL_DIR)/src/core/kernel.cpp \
    $(LOCAL_DIR)/src/core/lockable.cpp \
    $(LOCAL_DIR)/src/utils/memory_management.cpp \
    $(LOCAL_DIR)/src/primitives/mutex.cpp \
    $(LOCAL_DIR)/src/platform.cpp \
    $(LOCAL_DIR)/src/primitives/spinlock.cpp \
    $(LOCAL_DIR)/src/core/syscalls.cpp \
    $(LOCAL_DIR)/src/utils/linked_list.c

LOCAL_EXPORTED_DIRS := \
    $(LOCAL_DIR)/inc
LOCAL_ARFLAGS := -rcs

LOCAL_ARM_ARCHITECTURE := v7-m
LOCAL_ARM_FPU := nofp
LOCAL_COMPILER := arm_clang
LOCAL_STATIC_LIBS := \
    libpostform

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

