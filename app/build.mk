LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)
LOCAL_NAME := popcorn_stm32f103

LOCAL_CFLAGS := \
    $(TARGET_CFLAGS) \
    -I$(LOCAL_DIR)/inc

LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    $(TARGET_CXXFLAGS)

LOCAL_LDFLAGS := \
    -Wl,--gc-sections \
    -lnosys

LOCAL_LINKER_FILE := \
    $(LOCAL_DIR)/memory.ld

LOCAL_SRC := \
    $(LOCAL_DIR)/src/main.cpp \
    $(LOCAL_DIR)/src/system_stm32f1xx.c

LOCAL_ARM_ARCHITECTURE := v7-m
LOCAL_ARM_FPU := nofp
LOCAL_COMPILER := arm_clang

LOCAL_STATIC_LIBS := \
    libcortex_m_startup \
    libstm32cubef1 \
    libpopcorn \
    libpostform

include $(BUILD_BINARY)

