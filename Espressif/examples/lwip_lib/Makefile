# Feb 14 2015
# The current process is
# 	Remove Current Copy of SDK Headers , 
#	get Current SDK Headers , 
#   Make the Lib , 
#	Overwrite /Backup existing liblwip.a in SDK
#	Write headers used to a folder

XTENSA_TOOLS_ROOT 	?= c:/Espressif/xtensa-lx106-elf/bin
SDK_BASE		?= c:/Espressif/ESP8266_SDK

include ./Makefile-local.mk

# I use a separate DIR for the includes that are not part of the official SDK
USER_INC_BASE		?= C:/Espressif/ThirdParty/include

CC := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy

REQ_SDK_FOLDER = ./espressif/req_sdk_headers

SDK_LIB_DIR ?= $(SDK_BASE)/lib
SDK_INCDIR	?= $(SDK_BASE)/include
BACKUP_FILENAME ?= $(SDK_LIB_DIR)/org_liblwip.a
LIB_FILENAME    ?= $(SDK_LIB_DIR)/liblwip.a
#SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

# The dir structuredoesn't play nice with eclipse so added -Isrc/include/ipv4/  
CFLAGS = -Os -mlongcalls  -Isrc/include/ipv4/ -Isrc/include/ -Iconfig/ -Iespressif/include/
CFLAGS += -I$(REQ_SDK_FOLDER)/
CFLAGS += -DLWIP_OPEN_SRC
CFLAGS += -D__ets__

ifneq ($(LIBMAIN_PATH),)
    ADDITIONAL_TARGETS += replace_libmain
    CFLAGS += -DLWIP_OUR_LWIP_IF
else
    ifeq ($(USE_OUR_LWIP_IF),1)
	OBJS += our/eagle_lwip_if.o
	CFLAGS += -DLWIP_OUR_IF
    endif
endif

OBJS = \
src/api/api_lib.o \
src/api/api_msg.o \
src/api/err.o \
src/api/netbuf.o \
src/api/netdb.o \
src/api/netifapi.o \
src/api/sockets.o \
src/api/tcpip.o \
src/core/ipv4/autoip.o \
src/core/ipv4/icmp.o \
src/core/ipv4/igmp.o \
src/core/ipv4/inet.o \
src/core/ipv4/inet_chksum.o \
src/core/ipv4/ip.o \
src/core/ipv4/ip_addr.o \
src/core/ipv4/ip_frag.o \
src/core/dhcp.o \
src/core/dns.o \
src/core/def.o \
src/core/init.o \
src/core/netif.o \
src/core/mem.o \
src/core/memp.o \
src/core/pbuf.o \
src/core/raw.o \
src/core/stats.o \
src/core/sys.o \
src/core/tcp.o \
src/core/tcp_in.o \
src/core/tcp_out.o \
src/core/timers.o \
src/core/udp.o \
src/netif/etharp.o \
espressif/espconn.o \
espressif/espconn_tcp.o \
espressif/espconn_udp.o \
espressif/sys_arch.o \
espressif/netio.o \
espressif/dhcpserver.o \
espressif/ping.o

.PHONY: all clean \
remove_user_includes \
make_liblwip \
copy_liblwip_to_sdk_lib_dir \
create_user_include_dirs \
copy_includes_to_user_include_dirs \
get_includes_from_sdk \
replace_libmain


%.o: %.c
	$(CC) -c  $(CFLAGS) -o $@ $<
#	$(CC) -c  -I$(SDK_INCDIR) $(CFLAGS) -o $@ $<
	$(OBJCOPY) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $@

all: make_liblwip $(ADDITIONAL_TARGETS) copy_liblwip_to_sdk_lib_dir copy_includes_to_user_include_dirs

make_liblwip: clean get_includes_from_sdk liblwip.a
	
liblwip.a: $(OBJS)
	$(AR) rcs liblwip.a $(OBJS)
	
copy_liblwip_to_sdk_lib_dir: 
ifeq ("$(wildcard $(BACKUP_FILENAME))","")
	cp $(LIB_FILENAME) $(BACKUP_FILENAME)
	$(vecho) "Created Backup $(BACKUP_FILENAME)"
endif
	cp liblwip.a $(LIB_FILENAME)

create_user_include_dirs:
	-mkdir -p $(USER_INC_BASE)/pp
	-mkdir -p $(USER_INC_BASE)/netif
	-mkdir -p $(USER_INC_BASE)/arch
	-mkdir -p $(USER_INC_BASE)/lwip
	-mkdir -p $(USER_INC_BASE)/lwip/app
	
copy_includes_to_user_include_dirs: create_user_include_dirs
	cp ./src/include/ipv4/lwip/*.* $(USER_INC_BASE)/lwip/
	cp ./src/include/lwip/*.* $(USER_INC_BASE)/lwip/
	cp ./espressif/include/lwip/app/*.* $(USER_INC_BASE)/lwip/app/
	cp ./src/include/netif/*.* $(USER_INC_BASE)/netif/	
	cp ./espressif/include/arch/*.* $(USER_INC_BASE)/arch/
	cp ./espressif/include/pp/*.* $(USER_INC_BASE)/pp/
	cp ./espressif/include/mem_manager.h $(USER_INC_BASE)/
	cp ./config/lwipopts.h $(USER_INC_BASE)/

get_includes_from_sdk:
	cp $(SDK_INCDIR)/eagle_soc.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/ets_sys.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/osapi.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/queue.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/spi_flash.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/user_interface.h $(REQ_SDK_FOLDER)/
	cp $(SDK_INCDIR)/os_type.h $(REQ_SDK_FOLDER)/

#-Isrc/include/ -Iconfig/ -Iespressif/include/
	
remove_user_includes:
	rm -rf $(USER_INC_BASE)/pp
	rm -rf $(USER_INC_BASE)/netif
	rm -rf $(USER_INC_BASE)/arch
	rm -rf $(USER_INC_BASE)/lwip/app
	rm -rf $(USER_INC_BASE)/lwip
	
clean:
	rm -rf $(OBJS) liblwip.a
	rm -rf $(REQ_SDK_FOLDER)
	rm -rf our/eagle_lwip_if.o
	-mkdir -p $(REQ_SDK_FOLDER)
	
replace_libmain: our/eagle_lwip_if.o
	$(AR) rs $(LIBMAIN_PATH) $^
