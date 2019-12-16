LOCAL_DIR := $(call current-dir)

CC := gcc
CXX := g++

GLOBAL_CFLAGS := \
	-mthumb \
	-mcpu=cortex-m3 \
	-DSTM32F103xB \
	-Os \
	-g3 \
	-Wall \
	-Werror \
	-ffunction-sections \
	-fdata-sections

include $(CLEAR_VARS)
LOCAL_CROSS_COMPILE := arm-none-eabi-
LOCAL_NAME := stm32f1_fw.elf
LOCAL_CFLAGS := \
	$(GLOBAL_CFLAGS) \
	-I$(LOCAL_DIR)/Inc
LOCAL_CXXFLAGS := \
	$(LOCAL_CFLAGS) \
	-fno-exceptions \
	-fno-rtti
LOCAL_LDFLAGS := \
	-Wl,--gc-sections \
	--specs=nano.specs \
	--specs=nosys.specs
LOCAL_LINKER_FILE := \
	$(LOCAL_DIR)/STM32F103XB_FLASH.ld
LOCAL_SRC := \
	$(wildcard $(LOCAL_DIR)/Src/*.c) \
	$(wildcard $(LOCAL_DIR)/Src/*.cpp)
LOCAL_STATIC_LIBS := libstm32cubef1
include $(BUILD_BINARY)

include $(CLEAR_VARS)
LOCAL_CROSS_COMPILE := arm-none-eabi-
LOCAL_NAME := stm32cubef1
LOCAL_CFLAGS := \
	$(GLOBAL_CFLAGS) \
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