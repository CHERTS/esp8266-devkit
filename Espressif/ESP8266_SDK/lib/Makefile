XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin
SDK_BASE ?= c:/Espressif/ESP8266_SDK

CC := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar

SDK_LIB_DIR ?= $(SDK_BASE)/lib

CFLAGS = -Os -mlongcalls

OBJS = empty_user_rf_pre_init.o

.PHONY: all clean

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: make_libmain

make_libmain: clean libmain.a
	
libmain.a: $(OBJS)
	cp libmainorig.a libmain.a
	$(AR) r libmain.a $(OBJS)

clean:
	rm -rf $(OBJS)
	rm -rf libmain.a
	cp libmainorig.a libmain.a
