LOCAL_DIR := $(call current-dir)

CC := gcc
CXX := g++
LD := g++

TARGET_CFLAGS := \
	-mthumb \
	-mcpu=cortex-m3 \
	-DSTM32F103xB \
	$(GLOBAL_CFLAGS)

include $(CLEAR_VARS)
LOCAL_CROSS_COMPILE := arm-none-eabi-
LOCAL_NAME := stm32f1_fw.elf
LOCAL_CFLAGS := \
	$(TARGET_CFLAGS) \
	-I$(LOCAL_DIR)/. \
	-I$(LOCAL_DIR)/Inc
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS) \
	-fno-exceptions \
	-fno-rtti
LOCAL_LDFLAGS := \
	-Wl,--gc-sections \
	--specs=nano.specs \
	--specs=nosys.specs \
	-lstdc++
LOCAL_LINKER_FILE := \
	$(LOCAL_DIR)/STM32F103X8_FLASH.ld
LOCAL_SRC := \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)
LOCAL_STATIC_LIBS := libstm32cubef1 libos
include $(BUILD_BINARY)

include $(CLEAR_VARS)
LOCAL_CROSS_COMPILE := arm-none-eabi-
LOCAL_NAME := stm32cubef1
LOCAL_CFLAGS := \
	$(TARGET_CFLAGS) \
	-I$(LOCAL_DIR)/Inc \
	-I$(LOCAL_DIR)/STM32CubeF1/Drivers/CMSIS/Core/Include \
	-I$(LOCAL_DIR)/STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include \
	-I$(LOCAL_DIR)/STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Inc
LOCAL_ARFLAGS := -rcs
LOCAL_SRC := \
	$(filter-out %_template.c, $(wildcard $(LOCAL_DIR)/STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Src/*.c))
LOCAL_EXPORTED_DIRS := \
	$(LOCAL_DIR)/STM32CubeF1/Drivers/STM32F1xx_HAL_Driver/Inc \
	$(LOCAL_DIR)/STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include \
	$(LOCAL_DIR)/STM32CubeF1/Drivers/CMSIS/Core/Include

include $(BUILD_STATIC_LIB)

# Build GoogleTest
include $(CLEAR_VARS)
LOCAL_NAME := googletest
LOCAL_CFLAGS := \
	-Igoogletest/googletest/include \
	-Igoogletest/googlemock/include \
	-Igoogletest/googletest/ \
	-Igoogletest/googlemock/ \
	-m32 \
	-Os \
	-g3
LOCAL_CXXFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORTED_DIRS := \
	googletest/googletest/include \
	googletest/googlemock/include
LOCAL_ARFLAGS := -rcs
LOCAL_LDFLAGS := \
	-pthread \
	-lstdc++
LOCAL_SRC := \
	googletest/googletest/src/gtest.cc \
	googletest/googletest/src/gtest-death-test.cc \
	googletest/googletest/src/gtest-filepath.cc \
	googletest/googletest/src/gtest-matchers.cc \
	googletest/googletest/src/gtest-port.cc \
	googletest/googletest/src/gtest-printers.cc \
	googletest/googletest/src/gtest-test-part.cc \
	googletest/googletest/src/gtest-typed-test.cc \
	googletest/googlemock/src/gmock.cc \
	googletest/googlemock/src/gmock-cardinalities.cc \
	googletest/googlemock/src/gmock-matchers.cc \
	googletest/googlemock/src/gmock-internal-utils.cc \
	googletest/googlemock/src/gmock-spec-builders.cc \
	googletest/googletest/src/gtest_main.cc
include $(BUILD_STATIC_LIB)


