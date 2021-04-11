
# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = BlueRidge


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os


#######################################
# paths
#######################################
# Build path
BUILD_DIR = Build


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F407xx


# AS includes
AS_INCLUDES =


# ASM sources
ASM_SOURCES =  \
Src/startup_stm32f407xx.s


# C includes
C_INCLUDES =  \
-ISrc \
-ISrc/kernel/core \
-ISrc/kernel/cmsis/Include \
-ISrc/kernel/cmsis/Device/ST/STM32F4xx/Include \
-ISrc/kernel/hal-driver/Inc \
-ISrc/kernel/hal-driver/Inc/Legacy \
-ISrc/user/include


# C sources
C_SOURCES =  \
Src/system_stm32f4xx.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_rcc.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_rcc_ex.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_cortex.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_pwr.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_pwr_ex.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_dma.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_dma_ex.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_exti.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_flash.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_flash_ex.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_flash_ramfunc.c \
Src/kernel/hal-driver/Src/stm32f4xx_hal_gpio.c \
Src/driver/gpio/led.c \
Src/user/stm32f4xx_hal_msp.c \
Src/user/stm32f4xx_it.c \
Src/user/user_main.c



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections


ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2

C_INCLUDES += \
-ISrc/lib/

C_SOURCES += \
Src/lib/Segger/RTT/SEGGER_RTT_printf.c \
Src/lib/Segger/RTT/SEGGER_RTT.c

ASM_SOURCES += \
Src/lib/Segger/RTT/SEGGER_RTT_ASM_ARMv7M.s
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F407ZGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)/*

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***