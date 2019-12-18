BUILD_SYSTEM_DIR := buildsystem
include $(BUILD_SYSTEM_DIR)/top.mk

include build.mk
include $(call all-makefiles-under, .)

flash: stm32f1_fw.elf
	openocd -f "stm32_bluepill.cfg" -c "program build/targets/$< reset exit"
