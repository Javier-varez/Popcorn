BUILD_SYSTEM_DIR := buildsystem
include $(BUILD_SYSTEM_DIR)/top.mk

PREREQUISITES_OK := true
ARM_GCC_VERSION := 9.2.1

GLOBAL_CFLAGS := \
	-Os \
	-g3 \
	-Wall \
	-Werror \
	-ffunction-sections \
	-fdata-sections

INSTALLED_ARM_GCC_VERSION := \
	$(strip $(shell arm-none-eabi-gcc -dumpversion))
ifneq ($(INSTALLED_ARM_GCC_VERSION), $(ARM_GCC_VERSION))
$(warning arm-none-eabi-gcc doesn't match version $(ARM_GCC_VERSION). Detected: $(INSTALLED_ARM_GCC_VERSION))
$(warning skipping build for target)
PREREQUISITES_OK := false
endif

include build.mk
include $(call all-makefiles-under, .)

flash: stm32f1_fw.elf
	openocd -f "stm32_bluepill.cfg" -c "program build/targets/$< reset exit"

CPPLINT_DIRS := \
	OS \
	Src \
	Inc \
	Test

lint:
	find $(CPPLINT_DIRS) -name "*.cpp" -o -name "*.h" -o -name "*.c" | xargs cpplint