XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin
SDK_BASE ?= c:/Espressif/ESP8266_SDK

CC := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar

SDK_LIB_DIR ?= $(SDK_BASE)/lib
MAIN_LIB_FILENAME ?= $(SDK_LIB_DIR)/libmain.a

CFLAGS = -Os -mlongcalls

OBJS = empty_user_rf_pre_init.o

.PHONY: all clean

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: make_libmain

make_libmain: clean libmain.a
	
libmain.a: $(OBJS)
	cp libmain.a libmain.a.orig
	$(AR) r $(MAIN_LIB_FILENAME) $(OBJS)

clean:
	rm -rf $(OBJS)
	rm -rf libmain.a
	cp libmain.a.orig libmain.a
