LOCAL_DIR := $(call current-dir)

CC := g++
CXX := g++
LD := g++

PREREQUISITES_OK := true

ifeq ($(GOOGLETEST_LIBS_DIR),)
$(warning missing googletest/googlemock libraries. Skip $(LOCAL_DIR))
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
	-I$(GOOGLEMOCK_INCLUDE_DIR) \
	-I$(GOOGLETEST_INCLUDE_DIR) \
	$(GLOBAL_CFLAGS)
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS)
LOCAL_SRC := \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)

LOCAL_LDFLAGS := \
	-L$(GOOGLETEST_LIBS_DIR) \
	-lgtest \
	-lgtest_main \
	-lpthread
include $(BUILD_BINARY)
endif