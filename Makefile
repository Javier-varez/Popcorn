BUILD_SYSTEM_DIR := buildsystem
include $(BUILD_SYSTEM_DIR)/top.mk

GLOBAL_CFLAGS := \
	-Os \
	-g3 \
	-Wall \
	-Werror \
	-ffunction-sections \
	-fdata-sections

include build.mk
include $(call all-makefiles-under, .)

flash: stm32f1_fw.elf
	openocd -f "stm32_bluepill.cfg" -c "program build/targets/$< reset exit"

CPPLINT_DIRS := \
	OS \
	Src \
	Inc \
	Test \
	SystemTest

lint:
	find $(CPPLINT_DIRS) -name "*.cpp" -o -name "*.h" -o -name "*.c" | xargs cpplint
.PHONY: lint

