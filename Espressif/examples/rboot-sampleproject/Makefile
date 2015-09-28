#
# Makefile for rBoot sample project
# https://github.com/raburton/esp8266
#

# use wifi settings from environment or hard code them here
WIFI_SSID ?= dd-wrt
WIFI_PWD  ?= ESP8266@

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

#SDK_BASE   ?= /opt/esp-open-sdk/sdk
SDK_BASE	?= c:/Espressif/ESP8266_SDK
SDK_LIBDIR  = lib
SDK_INCDIR  = include

ESPTOOL2     ?= esptool2.exe
FW_SECTS      = .text .data .rodata
FW_USER_ARGS  = -quiet -bin -boot2

# select which tools to use as compiler, librarian and linker
CC	:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR	:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD	:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc

BUILD_DIR = build
FIRMW_DIR = firmware

SDK_LIBDIR := $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR := $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

LIBS    = c gcc hal phy net80211 lwip wpa main pp
CFLAGS  = -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wno-implicit -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls  -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

SRC		:= $(wildcard *.c)
OBJ		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
LIBS	:= $(addprefix -l,$(LIBS))

ifneq ($(WIFI_SSID), "")
	CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\"
endif
ifneq ($(WIFI_PWD), "")
	CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\"
endif

.SECONDARY:
.PHONY: all clean

C_FILES = $(wildcard *.c)
O_FILES = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_FILES))

all: $(BUILD_DIR) $(FIRMW_DIR) $(FIRMW_DIR)/rom0.bin $(FIRMW_DIR)/rom1.bin

$(BUILD_DIR)/%.o: %.c %.h
	@echo "CC $<"
	@$(CC) -I. $(SDK_INCDIR) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.elf: $(O_FILES)
	@echo "LD $(notdir $@)"
	@$(LD) -L$(SDK_LIBDIR) -T$(notdir $(basename $@)).ld $(LDFLAGS) -Wl,--start-group $(LIBS) $^ -Wl,--end-group -o $@

$(FIRMW_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "FW $(notdir $@)"
	@$(ESPTOOL2) $(FW_USER_ARGS) $^ $@ $(FW_SECTS)

$(BUILD_DIR):
	@mkdir -p $@

$(FIRMW_DIR):
	@mkdir -p $@

# esptool path and port
SDK_TOOLS	?= c:/Espressif/utils
ESPTOOL		?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM1
ESPBAUD		?= 115200
flash: all
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash -fs 8m 0x00000 $(FIRMW_DIR)/rboot.bin 0x02000 $(FIRMW_DIR)/rom0.bin 0x82000 $(FIRMW_DIR)/rom1.bin 0xfc000 $(FIRMW_DIR)/blank4.bin

clean:
	@echo "RM $(BUILD_DIR) $(FIRMW_DIR)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(FIRMW_DIR)
