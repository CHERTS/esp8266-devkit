#
# Makefile for all buildable projects, inc. esptool2 and rBoot
# https://github.com/raburton/esp8266
#
# This parent Makefile configures environment variables in one place, using
# sub-make to pass settings to each component. You can also build components
# individually, but be sure to modify paths in each respective Makefile to
# match your environment.

# Rules:
# `all`	 compile esptool2 and copy it to the project bin dir, compile rboot,
#				 compile rboot-sampleproject (which automatically copies required
#				 headers and source files from rBoot)
# `clean` run clean on esptool2, rboot, and rboot-sampleproject with inherited
#				 configuration

FW_BASE		= firmware

ESPTOOL2      ?= $(abspath esptool2/esptool2)
SDK_BASE      ?= c:/Espressif/ESP8266_SDK
# XTENSA_BINDIR can be blank if already in $PATH
XTENSA_BINDIR ?= c:/Espressif/xtensa-lx106-elf/bin

# esptool path and port
SDK_TOOLS	?= c:/Espressif/utils/ESP8266
ESPTOOL		?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM3
ESPBAUD		?= 115200

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

# rboot SPI size (256K, 512K, 1M, 2M ,4M)
SPI_SIZE ?= 512K

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

export

.PHONY: all esptool2 rboot rboot-sampleproject clean

all: esptool2 rboot rboot-sampleproject

esptool2:
	@echo "Building esptool2 firmware tool"
	@$(MAKE) -C esptool2

rboot:
	@echo "Building rBoot boot loader"
	@$(MAKE) -C rboot

rboot-sampleproject/rboot.h: rboot/rboot.h
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject/rboot-api.h: rboot/appcode/rboot-api.h
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject/rboot-api.c: rboot/appcode/rboot-api.c
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject: rboot-sampleproject/rboot.h rboot-sampleproject/rboot-api.h rboot-sampleproject/rboot-api.c
	@echo "Building rBoot sample project"
	@$(MAKE) -C rboot-sampleproject

flash-rboot: esptool2 rboot
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash $(flashimageoptions) 0x00000 rboot/$(FW_BASE)/rboot.bin

flash-sampleproject: esptool2 rboot-sampleproject
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash $(flashimageoptions) 0x02000 rboot-sampleproject/$(FW_BASE)/rom0.bin
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash $(flashimageoptions) 0x82000 rboot-sampleproject/$(FW_BASE)/rom1.bin

flash-all: flash-rboot flash-sampleproject flash-init

rebuild: clean all

flash-init:
	$(ESPTOOL) -p $(ESPPORT) write_flash $(flashimageoptions) \
		0x3fc000 $(SDK_BASE)/bin/esp_init_data_default.bin \
		0xfc000 rboot-sampleproject/blank4.bin

clean:
	@$(MAKE) -C esptool2 clean
	@$(MAKE) -C rboot clean
	@$(MAKE) -C rboot-sampleproject clean
