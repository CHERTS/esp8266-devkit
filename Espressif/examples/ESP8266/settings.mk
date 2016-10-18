# build directory
BUILD_BASE	= build

# firmware directory
FW_BASE		= firmware

# name for the target project
TARGET		= app

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK
SDK_TOOLS	?= c:/Espressif/utils/ESP8266

# Extra libs, include and ld file
EXTRA_BASE	?= c:/Espressif/extra

# esptool path and port
ESPTOOL		?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM3

# Baud rate for programmer
ESPBAUD		?= 256000

# BOOT = none
# BOOT = old - boot_v1.1
# BOOT = new - boot_v1.2+
BOOT ?= none
# APP = 0 - eagle.flash.bin + eagle.irom0text.bin
# APP = 1 - user1.bin
# APP = 2 - user2.bin
APP ?= 0
# SPI_SPEED = 40, 26, 20, 80
SPI_SPEED ?= 40
# SPI_MODE: QIO, QOUT, DIO, DOUT
SPI_MODE ?= QIO
# SPI_SIZE_MAP
# 0 : 512 KB (256 KB + 256 KB)
# 1 : 256 KB
# 2 : 1024 KB (512 KB + 512 KB)
# 3 : 2048 KB (512 KB + 512 KB)
# 4 : 4096 KB (512 KB + 512 KB)
# 5 : 2048 KB (1024 KB + 1024 KB)
# 6 : 4096 KB (1024 KB + 1024 KB)
SPI_SIZE_MAP ?= 2
