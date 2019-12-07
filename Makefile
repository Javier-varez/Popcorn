CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AR := arm-none-eabi-ar
SIZE := arm-none-eabi-size

STM32_CUBE_DIR := $(STM32CUBEF1_DIR)
LIB_OUT_DIR := build/lib
OUT_DIR := build
LIB_TARGET := $(LIB_OUT_DIR)/libstm32f1cube.a
TARGET := $(OUT_DIR)/stm32f1_fw.elf
DEFINES := -DSTM32F103xB
CPU_C_FLAGS := -mthumb -mcpu=cortex-m3

C_INCLUDE_DIRS := \
	$(STM32_CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32F1xx/Include \
	$(STM32_CUBE_DIR)/Drivers/CMSIS/Include \
	$(STM32_CUBE_DIR)/Drivers/STM32F1xx_HAL_Driver/Inc \
	Inc

C_LIB_SRC_DIR := \
	$(STM32_CUBE_DIR)/Drivers/STM32F1xx_HAL_Driver/Src

SRC_DIRS := \
	Src

CPU_C_FLAGS := -mcpu=cortex-m3 -mthumb

C_FLAGS := \
	$(addprefix -I, $(C_INCLUDE_DIRS)) \
	-DSTM32F103xB $(CPU_C_FLAGS) \
	-g3 -Os \
	-ffunction-sections \
	-fdata-sections \
	-Wl,--gc-sections

CXX_FLAGS := $(C_FLAGS) \
	-fno-exceptions \
	-fno-rtti

LD_FLAGS := \
	-L$(dir $(LIB_TARGET)) \
	-l stm32f1cube \
	-T STM32F103XB_FLASH.ld \
	--specs=nano.specs \
	--specs=nosys.specs

C_SOURCES := $(shell find $(SRC_DIRS) -name "*.c")
CPP_SOURCES := $(shell find $(SRC_DIRS) -name "*.cpp")
S_SOURCES := $(shell find $(SRC_DIRS) -name "*.s")
C_OBJECTS := $(addprefix $(OUT_DIR)/, $(patsubst %.c, %.o, $(C_SOURCES)))
CPP_OBJECTS := $(addprefix $(OUT_DIR)/, $(patsubst %.cpp, %.o, $(CPP_SOURCES)))
S_OBJECTS := $(addprefix $(OUT_DIR)/, $(patsubst %.s, %.o, $(S_SOURCES)))

C_LIB_SOURCES := $(shell find $(C_LIB_SRC_DIR) -name "*.c")
C_LIB_OBJECTS := $(addprefix $(LIB_OUT_DIR)/, $(notdir $(patsubst %.c, %.o, $(filter-out %template.c, $(C_LIB_SOURCES)))))

all: $(TARGET)
.PHONY: all

$(LIB_TARGET): $(C_LIB_OBJECTS)
	$(AR) -rcs $@ $^

$(LIB_OUT_DIR)/%.o: $(C_LIB_SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $(C_FLAGS) -o $@ $^

$(TARGET): $(LIB_TARGET) $(CPP_OBJECTS) $(C_OBJECTS) $(S_OBJECTS)
	$(CC) $(C_FLAGS) -o $@ $(C_OBJECTS) $(CPP_OBJECTS) $(S_OBJECTS) $(LD_FLAGS)
	$(SIZE) $@

$(OUT_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $(C_FLAGS) -o $@ $^

$(OUT_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $(CXX_FLAGS) -o $@ $^

$(OUT_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(C_FLAGS) -o $@ $^

clean:
	rm -r $(OUT_DIR)
.PHONY: clean
