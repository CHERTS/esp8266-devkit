#############################################################
#
# Root Level Makefile
#
# (c) by CHERTS <sleuthhound@gmail.com>
#
#############################################################

FW_BASE		= firmware

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK_095

SDK_TOOLS	?= c:/Espressif/utils
ESPTOOL		?= ../tools/esptool.exe
ESPTOOL_ORIG 	?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM3
ESPBAUD		?= 256000

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

# select which tools to use as compiler, librarian and linker
CC := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
NM := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-nm
CPP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-cpp
OBJCOPY := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OBJDUMP := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump
CCFLAGS += -Os -std=gnu90 -ffunction-sections -fno-jump-tables -fdata-sections

CSRCS ?= $(wildcard *.c)
ASRCs ?= $(wildcard *.s)
ASRCS ?= $(wildcard *.S)
SUBDIRS ?= $(patsubst %/,%,$(dir $(wildcard */Makefile)))

ODIR := .output
OBJODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/obj

OBJS := $(CSRCS:%.c=$(OBJODIR)/%.o) \
        $(ASRCs:%.s=$(OBJODIR)/%.o) \
        $(ASRCS:%.S=$(OBJODIR)/%.o)

DEPS := $(CSRCS:%.c=$(OBJODIR)/%.d) \
        $(ASRCs:%.s=$(OBJODIR)/%.d) \
        $(ASRCS:%.S=$(OBJODIR)/%.d)

LIBODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/lib
OLIBS := $(GEN_LIBS:%=$(LIBODIR)/%)

IMAGEODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/image
OIMAGES := $(GEN_IMAGES:%=$(IMAGEODIR)/%)

BINODIR := $(ODIR)/$(TARGET)/$(FLAVOR)/bin
OBINS := $(GEN_BINS:%=$(BINODIR)/%)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

CCFLAGS += 			\
	-g			\
	-O2			\
	-Wpointer-arith		\
	-Wundef			\
	-Werror			\
	-Wl,-EL			\
	-fno-inline-functions	\
	-nostdlib       \
	-mlongcalls	\
	-mtext-section-literals
#	-Wall			

CFLAGS = $(CCFLAGS) $(DEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)
DFLAGS = $(CCFLAGS) $(DDEFINES) $(EXTRA_CCFLAGS) $(INCLUDES)

define ShortcutRule
$(1): .subdirs $(2)/$(1)
endef

define MakeLibrary
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(LIBODIR)/$(1).a: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(LIBODIR)
	$$(if $$(filter %.a,$$?),mkdir -p $$(EXTRACT_DIR)_$(1))
	$$(if $$(filter %.a,$$?),cd $$(EXTRACT_DIR)_$(1); $$(foreach lib,$$(filter %.a,$$?),$$(AR) xo $$(UP_EXTRACT_DIR)/$$(lib);))
	$$(AR) ru $$@ $$(filter %.o,$$?) $$(if $$(filter %.a,$$?),$$(EXTRACT_DIR)_$(1)/*.o)
	$$(if $$(filter %.a,$$?),$$(RM) -r $$(EXTRACT_DIR)_$(1))
endef

define MakeImage
DEP_LIBS_$(1) = $$(foreach lib,$$(filter %.a,$$(COMPONENTS_$(1))),$$(dir $$(lib))$$(LIBODIR)/$$(notdir $$(lib)))
DEP_OBJS_$(1) = $$(foreach obj,$$(filter %.o,$$(COMPONENTS_$(1))),$$(dir $$(obj))$$(OBJODIR)/$$(notdir $$(obj)))
$$(IMAGEODIR)/$(1).out: $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1)) $$(DEPENDS_$(1))
	@mkdir -p $$(IMAGEODIR)
	$$(CC) $$(LDFLAGS) $$(if $$(LINKFLAGS_$(1)),$$(LINKFLAGS_$(1)),$$(LINKFLAGS_DEFAULT) $$(OBJS) $$(DEP_OBJS_$(1)) $$(DEP_LIBS_$(1))) -o $$@ 
endef

$(BINODIR)/%.bin: $(IMAGEODIR)/%.out
	@mkdir -p $(BINODIR)
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Section info:"
	$(Q) $(OBJDUMP) -h -j .data -j .rodata -j .bss -j .text -j .irom0.text $<
	$(vecho) "------------------------------------------------------------------------------"
	$(Q) $(ESPTOOL) elf2image $< -o../$(FW_BASE)/ $(flashimageoptions)
	$(vecho) "------------------------------------------------------------------------------"
	$(vecho) "Generate 0x00000.bin and 0x10000.bin successully in folder $(FW_BASE)."
	$(vecho) "0x00000.bin-------->0x00000"
	$(vecho) "0x10000.bin-------->0x10000"
	$(vecho) "Done"

all: .subdirs $(OBJS) $(OLIBS) $(OIMAGES) $(OBINS) $(SPECIAL_MKTARGETS)

clean:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clean;)
	$(RM) -r $(ODIR)/$(TARGET)/$(FLAVOR)
	$(RM) $(FW_BASE)/*.bin
	$(RM) app/mapfile

clobber: $(SPECIAL_CLOBBER)
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) clobber;)
	$(RM) -r $(ODIR)

flash:
	$(foreach d, $(SUBDIRS), $(MAKE) -C $(d) progr;)

progr: all
ifndef PDIR
	$(MAKE) -C ./app flash
else
	$(ESPTOOL) -p $(ESPPORT) -b $(ESPBAUD) write_flash $(flashimageoptions) 0x00000 ../$(FW_BASE)/0x00000.bin 0x10000 ../$(FW_BASE)/0x10000.bin
endif

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
	$(ESPTOOL_ORIG) -p $(ESPPORT) write_flash $(flashimageoptions) \
		0x3e000 $(SDK_BASE)/bin/blank.bin \
		0x3fc000 $(SDK_BASE)/bin/esp_init_data_default.bin \
		0x3fe000 $(SDK_BASE)/bin/blank.bin

luainit:
	$(SDK_TOOLS)/nodemcutil.exe -p $(ESPPORT) -s init.lua
	$(SDK_TOOLS)/nodemcutil.exe -p $(ESPPORT) -rt

.subdirs:
	@set -e; $(foreach d, $(SUBDIRS), $(MAKE) -C $(d);)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clobber)
ifdef DEPS
sinclude $(DEPS)
endif
endif
endif

$(OBJODIR)/%.o: %.c
	@mkdir -p $(OBJODIR);
	$(CC) $(if $(findstring $<,$(DSRCS)),$(DFLAGS),$(CFLAGS)) $(COPTS_$(*F)) -o $@ -c $<

$(OBJODIR)/%.d: %.c
	@mkdir -p $(OBJODIR);
	@echo DEPEND: $(CC) -M $(CFLAGS) $<
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.s
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJODIR)/%.d: %.s
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJODIR)/%.o: %.S
	@mkdir -p $(OBJODIR);
	$(CC) $(CFLAGS) -D__ASSEMBLER__ -o $@ -c $<

$(OBJODIR)/%.d: %.S
	@mkdir -p $(OBJODIR); \
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\.o\)[ :]*,$(OBJODIR)/\1 $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(foreach lib,$(GEN_LIBS),$(eval $(call ShortcutRule,$(lib),$(LIBODIR))))

$(foreach image,$(GEN_IMAGES),$(eval $(call ShortcutRule,$(image),$(IMAGEODIR))))

$(foreach bin,$(GEN_BINS),$(eval $(call ShortcutRule,$(bin),$(BINODIR))))

$(foreach lib,$(GEN_LIBS),$(eval $(call MakeLibrary,$(basename $(lib)))))

$(foreach image,$(GEN_IMAGES),$(eval $(call MakeImage,$(basename $(image)))))

INCLUDES := $(INCLUDES) -I $(PDIR)include -I $(PDIR)include/$(TARGET)
