LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)
LOCAL_NAME := SystemTest
LOCAL_CFLAGS := \
	-Wall -Werror \
	-I$(LOCAL_DIR)/Inc
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS) \
	-std=c++17
LOCAL_SRC := \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)
# Set compilers and linker to g++
CC := g++
CXX := g++
LD := g++
include $(BUILD_BINARY)