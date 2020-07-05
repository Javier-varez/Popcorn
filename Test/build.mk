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
	@lcov --zerocounters --directory build/intermediates/TestOS/ --base-directory .
	@$<
	@lcov --capture --no-external --directory build/intermediates/TestOS/ --base-directory . --rc lcov_branch_coverage=1 --output-file $@
	@lcov --remove $@ --output-file $@ --rc lcov_branch_coverage=1 '$(INTERNAL_LOCAL_DIR)/Src/*' '$(INTERNAL_LOCAL_DIR)/Inc/*'
	@genhtml -o build/coverage/html --rc genhtml_branch_coverage=1 $@

TestCoverage: build/coverage/test_coverage.info
.PHONY += TestCoverage

endif
