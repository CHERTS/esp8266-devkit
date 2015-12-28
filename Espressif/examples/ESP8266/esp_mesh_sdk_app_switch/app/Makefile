#############################################################
# Required variables for each makefile
# Discard this section from all parent makefiles
# Expected variables (with automatic defaults):
#   CSRCS (all "C" files in the dir)
#   SUBDIRS (all subdirs with a Makefile)
#   GEN_LIBS - list of libs to be generated ()
#   GEN_IMAGES - list of object file images to be generated ()
#   GEN_BINS - list of binaries to be generated ()
#   COMPONENTS_xxx - a list of libs/objs in the form
#     subdir/lib to be extracted and rolled up into
#     a generated lib/image xxx.a ()
#
TARGET = eagle
FLAVOR = release
#FLAVOR = debug

#EXTRA_CCFLAGS += -u

ifndef PDIR # {
GEN_IMAGES= eagle.app.v6.out
GEN_BINS= eagle.app.v6.bin
SPECIAL_MKTARGETS=$(APP_MKTARGETS)
SUBDIRS=    \
	user    \
	driver

endif # } PDIR

APPDIR = .
LDDIR = $(SDK_BASE)/ld

CCFLAGS += -Os

TARGET_LDFLAGS =		\
	-nostdlib		\
	-Wl,-EL \
	--longcalls \
	--text-section-literals

ifeq ($(FLAVOR),debug)
    TARGET_LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
    TARGET_LDFLAGS += -g -O0
endif


dummy: all

libesphttpd/libesphttpd.a: libesphttpd/Makefile
	make -C libesphttpd checkdirs
	make -C libesphttpd libesphttpd.a

libesphttpd/libwebpages-espfs.a: libesphttpd/Makefile
	make -C libesphttpd checkdirs
	make -C libesphttpd libwebpages-espfs.a

LD_FILE = $(LDDIR)/eagle.app.v6.ld

COMPONENTS_eagle.app.v6 = \
	user/libuser.a  \
	driver/libdriver.a

LINKFLAGS_eagle.app.v6 = \
	-L$(SDK_BASE)/lib        \
	-nostdlib	\
	-T$(LD_FILE)   \
	-Wl,--no-check-sections	\
	-u call_user_start	\
	-Wl,-static						\
	-Wl,--start-group					\
	-lc					\
	-lmgcc					\
	-lhal					\
	-lphy	\
	-lpp	\
	-lnet80211	\
	-llwip	\
	-lwpa	\
	-lmain	\
	-lssl	\
	-ljson	\
	-lpwm \
	-lsmartconfig \
	-lwps \
	-lespnow \
	-lmesh \
	-L./libesphttpd \
	-lesphttpd \
	-lwebpages-espfs \
	$(DEP_LIBS_eagle.app.v6)					\
	-Wl,--end-group

DEPENDS_eagle.app.v6 = \
                $(LD_FILE) \
                $(LDDIR)/eagle.rom.addr.v6.ld \
		libesphttpd/libesphttpd.a \
		libesphttpd/libwebpages-espfs.a

#############################################################
# Configuration i.e. compile options etc.
# Target specific stuff (defines etc.) goes in here!
# Generally values applying to a tree are captured in the
#   makefile at its root level - these are then overridden
#   for a subtree within the makefile rooted therein
#

#UNIVERSAL_TARGET_DEFINES =		\

# Other potential configuration flags include:
#	-DTXRX_TXBUF_DEBUG
#	-DTXRX_RXBUF_DEBUG
#	-DWLAN_CONFIG_CCX
CONFIGURATION_DEFINES =	-DICACHE_FLASH

DEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)

DDEFINES +=				\
	$(UNIVERSAL_TARGET_DEFINES)	\
	$(CONFIGURATION_DEFINES)


#############################################################
# Recursion Magic - Don't touch this!!
#
# Each subtree potentially has an include directory
#   corresponding to the common APIs applicable to modules
#   rooted at that subtree. Accordingly, the INCLUDE PATH
#   of a module can only contain the include directories up
#   its parent path, and not its siblings
#
# Required for each makefile to inherit from the parent
#

INCLUDES := $(INCLUDES) -I $(PDIR)include -I $(PDIR)libesphttpd/include
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile

.PHONY: FORCE
FORCE:

