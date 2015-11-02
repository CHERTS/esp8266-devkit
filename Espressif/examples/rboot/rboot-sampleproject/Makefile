#
# Makefile for rBoot sample project
# https://github.com/raburton/esp8266
#

# use wifi settings from environment or hard code them here
WIFI_SSID ?= ""
WIFI_PWD  ?= ""

SDK_BASE   ?= /opt/esp-open-sdk/sdk
SDK_LIBDIR  = lib
SDK_INCDIR  = include

ESPTOOL2     ?= ../esptool2/esptool2
FW_SECTS      = .text .data .rodata
FW_USER_ARGS  = -quiet -bin -boot2

ifndef XTENSA_BINDIR
CC := xtensa-lx106-elf-gcc
LD := xtensa-lx106-elf-gcc
else
CC := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
LD := $(addprefix $(XTENSA_BINDIR)/,xtensa-lx106-elf-gcc)
endif

BUILD_DIR = build
FIRMW_DIR = firmware

SDK_LIBDIR := $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR := $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

LIBS    = c gcc hal phy net80211 lwip wpa main pp
CFLAGS  = -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wno-implicit -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls  -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

SRC		:= $(wildcard *.c)
OBJ		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))

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

clean:
	@echo "RM $(BUILD_DIR) $(FIRMW_DIR)"
	@rm -rf $(BUILD_DIR)
	@rm -rf $(FIRMW_DIR)
