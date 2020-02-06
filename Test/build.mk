LOCAL_DIR := $(call current-dir)

CC := g++
CXX := g++
LD := g++

PREREQUISITES_OK := true

ifeq ($(GOOGLETEST_LIBS32_DIR),)
$(warning missing googletest/googlemock 32 bit libraries. Skip $(LOCAL_DIR))
PREREQUISITES_OK := false
endif

ifeq ($(GOOGLEMOCK_INCLUDE_DIR),)
$(warning missing googletest include files. Skip $(LOCAL_DIR))
PREREQUISITES_OK := false
endif

ifeq ($(GOOGLETEST_INCLUDE_DIR),)
$(warning missing googlemock include files. Skip $(LOCAL_DIR))
PREREQUISITES_OK := false
endif

ifeq ($(PREREQUISITES_OK),true)
include $(CLEAR_VARS)
LOCAL_NAME := TestOS
LOCAL_CFLAGS := \
	-I$(LOCAL_DIR)/Inc \
	-I$(LOCAL_DIR)/../Inc \
	-I$(GOOGLEMOCK_INCLUDE_DIR) \
	-I$(GOOGLETEST_INCLUDE_DIR) \
	-DUNITTEST \
	-m32 \
	-Os \
	-g3 \
	-Wall \
	-Werror
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS)
TEST_SRC := \
	$(LOCAL_DIR)/../Src/syscalls.cpp \
	$(LOCAL_DIR)/../Src/cortex-m_port.cpp \
	$(LOCAL_DIR)/../Src/kernel.cpp \
	$(LOCAL_DIR)/../Src/linked_list.c

LOCAL_SRC := \
	$(TEST_SRC) \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)

LOCAL_LDFLAGS := \
	-L$(GOOGLETEST_LIBS32_DIR) \
	-lgtest \
	-lgtest_main \
	-lgmock \
	-lpthread
include $(BUILD_BINARY)
endif