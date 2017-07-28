# stlink folder so make burn will work.
STLINK=/opt/stlink/stlink/build/

# project name
PROJ_NAME=main

# Source files
SRCS = Device/startup_stm32f401xe.s #startup file
SRCS += Device/system_stm32f4xx.c #core clock setup
SRCS += Device/stm32f4xx_it.c #main interrupt service routines
SRCS += led_blink.c

# Location of the Libraries folder from the STM32F0xx Standard Peripheral Library
STD_PERIPH_LIB=./Drivers

# Location of the linker scripts
LDSCRIPT_INC=./Device/ldscripts

###################################################

CC=arm-none-eabi-gcc # arm compiler
OBJCOPY=arm-none-eabi-objcopy #objcopy to copy content of one object file to another http://www.thegeekstuff.com/2013/01/objcopy-examples/
OBJDUMP=arm-none-eabi-objdump #objdump to display informations about object file
SIZE=arm-none-eabi-size #size to display information on programming file size

###################################################
# CFLAGS

CFLAGS  = -ggdb -O0 -Wall -Wextra -Warray-bounds
CFLAGS += -mcpu=cortex-m4 -mthumb -mlittle-endian -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DSTM32F401xE
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map

###################################################

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS += -I inc -I $(STD_PERIPH_LIB) -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include -I $(STD_PERIPH_LIB)/STM32F4xx_HAL_Driver/Inc
CFLAGS += -include $(STD_PERIPH_LIB)/stm32f4xx_hal_conf.h

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C $(STD_PERIPH_LIB)

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstmf4 -L$(LDSCRIPT_INC) -Tnucleo.ld -g
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(OBJDUMP) -St $(PROJ_NAME).elf >$(PROJ_NAME).lst
	$(SIZE) $(PROJ_NAME).elf
	
clean:
	find ./ -name '*~' | xargs rm -f	
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst

cleanall: clean
	$(MAKE) -C $(STD_PERIPH_LIB) clean
	
# Flash the STM32F4 using STLINK
burn: proj
	$(STLINK)/st-flash write $(PROJ_NAME).bin 0x8000000
