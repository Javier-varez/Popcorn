LOCAL_DIR := $(call current-dir)

GLOBAL_CFLAGS := \
    -Os \
    -g3 \
    -Wall \
    -Werror \
    -ffunction-sections \
    -fdata-sections

TARGET_CFLAGS := \
	-mthumb \
	-mcpu=cortex-m3 \
	-DSTM32F103xB \
	$(GLOBAL_CFLAGS)


include $(call all-makefiles-under, $(LOCAL_DIR))

