#############################################################
#
# Root Level Makefile ESP8266 + RTOS
#
# (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

BUILD_BASE	= build
FW_BASE		= firmware

TOOLS_ROOT ?= c:/Espressif

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= $(TOOLS_ROOT)/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= $(TOOLS_ROOT)/ESP8266_RTOS_SDK

# esptool path and port
ESPTOOL		?= $(TOOLS_ROOT)/utils/esptool.exe
ESPPORT		?= COM2
ESPBAUD		?= 115200

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES	= user driver
EXTRA_INCDIR = include

#LIBS       = cirom gccirom hal phy pp net80211 wpa main freertos lwip udhcp
# linking libgccirom.a instead of libgcc.a causes reset when working with flash memory (ie spi_flash_erase_sector)
LIBS		= gcc hal phy pp net80211 wpa main freertos lwip udhcp

# compiler flags using during compilation of source files
CFLAGS		= -g -save-temps -Os -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
CXXFLAGS	= $(CFLAGS) -fno-rtti -fno-exceptions

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= extra_include include include/espressif include/json include/udhcp include/lwip include/lwip/lwip include/lwip/ipv4 include/lwip/ipv6

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
CXX		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-g++
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c*))
C_OBJ		:= $(patsubst %.c,%.o,$(SRC))
CXX_OBJ		:= $(patsubst %.cpp,%.o,$(C_OBJ))
OBJ		:= $(patsubst %.o,$(BUILD_BASE)/%.o,$(CXX_OBJ))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
$1/%.o: %.cpp
	$(vecho) "C+ $$<"
	$(Q) $(CXX) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CXXFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean iromlibs firmware flashonefile flash flashinit rebuild

all: checkdirs $(TARGET_OUT)

# create libgccirom.a from libgcc.a
# create libcirom.a from libc.a
iromlibs:
	@echo "Creating irom version of libgcc and libc..."
	$(Q) $(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $(TOOLS_ROOT)/xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/4.8.2/libgcc.a $(TOOLS_ROOT)/xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/4.8.2/libgccirom.a
	$(Q) $(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $(TOOLS_ROOT)/xtensa-lx106-elf/xtensa-lx106-elf/lib/libc.a $(TOOLS_ROOT)/xtensa-lx106-elf/xtensa-lx106-elf/lib/libcirom.a

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(vecho) "Run objcopy, please wait..."
	$(Q) $(OBJCOPY) --only-section .text -O binary $@ eagle.app.v6.text.bin
	$(Q) $(OBJCOPY) --only-section .data -O binary $@ eagle.app.v6.data.bin
	$(Q) $(OBJCOPY) --only-section .rodata -O binary $@ eagle.app.v6.rodata.bin
	$(Q) $(OBJCOPY) --only-section .irom0.text -O binary $@ eagle.app.v6.irom0text.bin
	$(vecho) "objcopy done"
	$(vecho) "Run gen_appbin.exe"
	$(TOOLS_ROOT)/utils/gen_appbin_old.exe $(TARGET_OUT) v6
	$(Q) mv eagle.app.v6.flash.bin $(FW_BASE)/eagle.flash.bin
	$(Q) mv eagle.app.v6.irom0text.bin $(FW_BASE)/eagle.irom0text.bin
	$(Q) rm eagle.app.v6.*
	$(Q) rm eagle.app.sym
	$(vecho) "Generate eagle.flash.bin and eagle.irom0text.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.flash.bin-------->0x00000"
	$(vecho) "eagle.irom0text.bin---->0x40000"
	$(Q) rm -f *.ii
	$(Q) rm -f *.i
	$(Q) rm -f *.s
	$(vecho) "Done"

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flashonefile: all
	$(OBJCOPY) --only-section .text -O binary $(TARGET_OUT) eagle.app.v6.text.bin
	$(OBJCOPY) --only-section .data -O binary $(TARGET_OUT) eagle.app.v6.data.bin
	$(OBJCOPY) --only-section .rodata -O binary $(TARGET_OUT) eagle.app.v6.rodata.bin
	$(OBJCOPY) --only-section .irom0.text -O binary $(TARGET_OUT) eagle.app.v6.irom0text.bin
	$(TOOLS_ROOT)/utils/gen_appbin_old.exe $(TARGET_OUT) v6
	$(TOOLS_ROOT)/utils/gen_flashbin.exe eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin 0x40000
	rm -f eagle.app.v6.data.bin
	rm -f eagle.app.v6.flash.bin
	rm -f eagle.app.v6.irom0text.bin
	rm -f eagle.app.v6.rodata.bin
	rm -f eagle.app.v6.text.bin
	rm -f eagle.app.sym
	mv eagle.app.flash.bin $(FW_BASE)/
	$(vecho) "No boot needed."
	$(vecho) "Generate eagle.app.flash.bin successully in folder $(FW_BASE)."
	$(vecho) "eagle.app.flash.bin-------->0x00000"
	$(Q) rm -f *.ii
	$(Q) rm -f *.i
	$(Q) rm -f *.s
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash 0x00000 $(FW_BASE)/eagle.app.flash.bin

flash: all
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash 0x00000 $(FW_BASE)/eagle.flash.bin 0x40000 $(FW_BASE)/eagle.irom0text.bin

flashinit:
	$(vecho) "Flash init data default and blank data."
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash 0x7c000 $(SDK_BASE)/bin/esp_init_data_default.bin 0x7e000 $(SDK_BASE)/bin/blank.bin

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -f *.bin
	$(Q) rm -f *.sym
	$(Q) rm -f *.ii
	$(Q) rm -f *.i
	$(Q) rm -f *.s
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
