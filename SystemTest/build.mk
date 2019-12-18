LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)
LOCAL_NAME := SystemTest
LOCAL_CFLAGS := \
	-Wall -Werror \
	-I$(LOCAL_DIR)/Inc
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS)
LOCAL_LDFLAGS := \
	-lstdc++
LOCAL_SRC := \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)
include $(BUILD_BINARY)