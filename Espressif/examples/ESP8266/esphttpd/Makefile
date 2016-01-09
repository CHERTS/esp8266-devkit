#Position and maximum length of espfs in flash memory. This can be undefined. In this case 
#the webpages will be linked in into the executable file. If this is defined, please do a 
#'make htmlflash' to flash the espfs into the ESPs memory.
ESPFS_POS = 0x12000
ESPFS_SIZE = 0x2E000

# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware

# Base directory for the compiler. Needs a / at the end; if not set it'll use the tools that are in
# the PATH.
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK
SDK_TOOLS	?= c:/Espressif/utils/ESP8266

# esptool path and port
ESPTOOL ?= $(SDK_TOOLS)/esptool.exe
ESPPORT ?= COM3
# Baud rate for programmer
BAUD ?= 256000

# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED ?= 40
# SPI_MODE: qio, qout, dio, dout
SPI_MODE ?= qio
# SPI_SIZE_MAP
# 0 : 512 KB (256 KB + 256 KB)
# 1 : 256 KB
# 2 : 1024 KB (512 KB + 512 KB)
# 3 : 2048 KB (512 KB + 512 KB)
# 4 : 4096 KB (512 KB + 512 KB)
# 5 : 2048 KB (1024 KB + 1024 KB)
# 6 : 4096 KB (1024 KB + 1024 KB)
SPI_SIZE_MAP ?= 0

ifeq ($(SPI_SPEED), 26.7)
    freqdiv = 1
    flashimageoptions = -ff 26m
else
    ifeq ($(SPI_SPEED), 20)
        freqdiv = 2
        flashimageoptions = -ff 20m
    else
        ifeq ($(SPI_SPEED), 80)
            freqdiv = 15
            flashimageoptions = -ff 80m
        else
            freqdiv = 0
            flashimageoptions = -ff 40m
        endif
    endif
endif

ifeq ($(SPI_MODE), QOUT)
    mode = 1
    flashimageoptions += -fm qout
else
    ifeq ($(SPI_MODE), DIO)
		mode = 2
		flashimageoptions += -fm dio
    else
        ifeq ($(SPI_MODE), DOUT)
            mode = 3
            flashimageoptions += -fm dout
        else
            mode = 0
            flashimageoptions += -fm qio
        endif
    endif
endif

ifeq ($(SPI_SIZE_MAP), 1)
  size_map = 1
  flash = 256
  flashimageoptions += -fs 2m
else
  ifeq ($(SPI_SIZE_MAP), 2)
    size_map = 2
    flash = 1024
    flashimageoptions += -fs 8m
  else
    ifeq ($(SPI_SIZE_MAP), 3)
      size_map = 3
      flash = 2048
      flashimageoptions += -fs 16m
    else
      ifeq ($(SPI_SIZE_MAP), 4)
		size_map = 4
		flash = 4096
		flashimageoptions += -fs 32m
      else
        ifeq ($(SPI_SIZE_MAP), 5)
          size_map = 5
          flash = 2048
          flashimageoptions += -fs 16m
        else
          ifeq ($(SPI_SIZE_MAP), 6)
            size_map = 6
            flash = 4096
            flashimageoptions += -fs 32m
          else
            size_map = 0
            flash = 512
            flashimageoptions += -fs 4m
          endif
        endif
      endif
    endif
  endif
endif

# name for the target project
TARGET		= httpd

# which modules (subdirectories) of the project to include in compiling
MODULES		= user
EXTRA_INCDIR	= include libesphttpd/include

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy pp net80211 wpa main lwip crypto
#Add in esphttpd lib
LIBS += esphttpd

# compiler flags using during compilation of source files
CFLAGS		= -Os -ggdb -std=c99 -Werror -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
		-nostdlib -mlongcalls -mtext-section-literals -mno-serialize-volatile -D__ets__ -DICACHE_FLASH -D_STDINT_H \
		-Wno-address

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump

####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
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

ifeq ("$(GZIP_COMPRESSION)","yes")
CFLAGS		+= -DGZIP_COMPRESSION
endif

ifeq ("$(USE_HEATSHRINK)","yes")
CFLAGS		+= -DESPFS_HEATSHRINK
endif

ifeq ("$(ESPFS_POS)","")
#No hardcoded espfs position: link it in with the binaries.
LIBS += -lwebpages-espfs
else
#Hardcoded espfs location: Pass espfs position to rest of code
CFLAGS += -DESPFS_POS=$(ESPFS_POS) -DESPFS_SIZE=$(ESPFS_SIZE)
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean libesphttpd

all: checkdirs $(TARGET_OUT)

libesphttpd/Makefile:
	$(Q) echo "No libesphttpd submodule found. Using git to fetch it..."
	$(Q) git submodule init
	$(Q) git submodule update

libesphttpd: libesphttpd/Makefile
	$(Q) make -C libesphttpd

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -Llibesphttpd -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	$(Q) $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $@
	$(vecho) "------------------------------------------------------------------------------"
	$(Q) $(ESPTOOL) elf2image $(TARGET_OUT) -o$(FW_BASE)/ $(flashimageoptions)
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Generate 0x00000.bin and 0x40000.bin successully in folder $(FW_BASE)."
	$(vecho) "0x00000.bin-------->0x00000"
	$(vecho) "0x40000.bin-------->0x40000"
	$(vecho) "Done"

$(FW_BASE):
	$(Q) mkdir -p $@

$(APP_AR): libesphttpd $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $(OBJ)

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

flash: $(TARGET_OUT) $(FW_BASE)
	$(ESPTOOL) -p $(ESPPORT) -b $(BAUD) write_flash $(flashimageoptions) 0x00000 $(FW_BASE)/0x00000.bin 0x40000 $(FW_BASE)/0x40000.bin

htmlflash: libesphttpd
	$(Q) if [ $$(stat -c '%s' libesphttpd/webpages.espfs) -gt $$(( $(ESPFS_SIZE) )) ]; then echo "webpages.espfs too big!"; false; fi
	$(ESPTOOL) -p $(ESPPORT) -b $(BAUD) write_flash $(flashimageoptions) $(ESPFS_POS) libesphttpd/webpages.espfs

# ===============================================================
# From http://bbs.espressif.com/viewtopic.php?f=10&t=305
# master-device-key.bin is only need if using espressive services
# master_device_key.bin 0x3e000 is not used , write blank
# See 2A-ESP8266__IOT_SDK_User_Manual__EN_v1.1.0.pdf
# http://bbs.espressif.com/download/file.php?id=532
#
# System parameter area is the last 16KB of flash
# 512KB flash - system parameter area starts from 0x7C000 
# 	download blank.bin to 0x7E000 as initialization.
# 1024KB flash - system parameter area starts from 0xFC000 
# 	download blank.bin to 0xFE000 as initialization.
# 2048KB flash - system parameter area starts from 0x1FC000 
# 	download blank.bin to 0x1FE000 as initialization.
# 4096KB flash - system parameter area starts from 0x3FC000 
# 	download blank.bin to 0x3FE000 as initialization.
# ===============================================================

# FLASH SIZE
flashinit:
	$(vecho) "Flash init data:"
	$(vecho) "Default config (Clear SDK settings):"
	$(vecho) "blank.bin-------->0x3e000"
	$(vecho) "blank.bin-------->0x3fc000"
	$(vecho) "esp_init_data_default.bin-------->0x3fc000"
	$(ESPTOOL) -p $(ESPPORT) write_flash $(flashimageoptions) \
		0x3e000 $(SDK_BASE)/bin/blank.bin \
		0x3fc000 $(SDK_BASE)/bin/esp_init_data_default.bin \
		0x3fe000 $(SDK_BASE)/bin/blank.bin

clean:
	$(Q) make -C libesphttpd clean
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) find $(BUILD_BASE) -type f | xargs rm -f
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
