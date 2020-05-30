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
	-I$(LOCAL_DIR)/../ \
	-I$(LOCAL_DIR)/../OS \
	-I$(GOOGLEMOCK_INCLUDE_DIR) \
	-I$(GOOGLETEST_INCLUDE_DIR) \
	-DUNITTEST \
	-m32 \
	-g3 \
	-Wall \
	-Werror \
	-fprofile-arcs \
	-ftest-coverage
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS)
TEST_SRC := \
	$(LOCAL_DIR)/../OS/Src/syscalls.cpp \
	$(LOCAL_DIR)/../OS/Src/cortex-m_port.cpp \
	$(LOCAL_DIR)/../OS/Src/kernel.cpp \
	$(LOCAL_DIR)/../OS/Src/linked_list.c \
	$(LOCAL_DIR)/../OS/Src/spinlock.cpp \
	$(LOCAL_DIR)/../OS/Src/mutex.cpp

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

# Test coverage generation
build/coverage/test_coverage.info: INTERNAL_LOCAL_DIR := $(PWD)/$(LOCAL_DIR)
build/coverage/test_coverage.info: build/targets/TestOS
	@mkdir -p $(dir $@)
	@$<
	@lcov --no-external --capture --directory build/intermediates/TestOS/ --base-directory . --output-file $@
	lcov --remove $@ --output-file $@ '$(INTERNAL_LOCAL_DIR)/Src/*' '$(INTERNAL_LOCAL_DIR)/Inc/*'
	@genhtml -o build/coverage/html $@

TestCoverage: build/coverage/test_coverage.info
.PHONY += TestCoverage

endif
