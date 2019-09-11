LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

GREEN="\e[32;1m"
DONE="\e[39m"
RED="\e[31m"

COMPILE_ROOT := $(LOCAL_DIR)
FFMPEG_ORG_VER := ffmpeg-3.4.1
FFMPEG_ORG_TAR := ffmpeg-3.4.1.tar.gz
FFMPEG_VER := ffmpeg-y
FFMPEG_TAR := ffmpeg-y.tgz
#3516cv300/3518ev300 sdk not support vdec, so have to use vdev of ffmpeg
ifeq ($(CFG_CHIP_TYPE), hi3516cv300)
FF_CONFIG_SH := enable_decoder_config.sh
FF_ENABLE_MINI=n
else ifeq ($(CFG_CHIP_TYPE), hi3518ev300)
FF_CONFIG_SH := enable_decoder_config.sh
FF_ENABLE_MINI=n
else
FF_CONFIG_SH := disable_decoder_config.sh
ifeq ($(CFG_ENABLE_MINI), y)
FF_ENABLE_MINI=y
else
FF_ENABLE_MINI=n
endif
endif

ifeq ($(CFG_OS_TYPE_LITEOS),y)
export LITEOS_MACRO
export LITEOS_OSDRV_INCLUDE
export LITEOS_USR_INCLUDE
export LITEOS_CMACRO
export LITEOS_INCLUDE
export LITEOSTOPDIR

#override CFLAGS to avoid ffmpeg configure use
CFLAGS:=
FF_ADAPT_LITEOS=y
else
FF_ADAPT_LITEOS=n
endif

ORG_FFMPEG := $(FFMPEG_VER)

MODNAME := $(FFMPEG_VER)

all: $(ORG_FFMPEG)

.PHONY: $(ORG_FFMPEG) clean

install:$(ORG_FFMPEG)

clean:
	rm -rf $(FFMPEG_VER);
	rm -rf $(FFMPEG_TAR);
	rm -rf $(FFMPEG_ORG_VER);
ffmpeg_download:
	@if [ ! -f $(FFMPEG_ORG_VER).tar.gz ]; then curl -O http://ffmpeg.org/releases/$(FFMPEG_ORG_VER).tar.gz; fi

install_patch: ffmpeg_download
	@if [ ! -f $(FFMPEG_TAR) -a -f $(FFMPEG_ORG_VER).tar.gz -a 'y' != $(FF_ENABLE_MINI) ]; then \
	tar xf $(FFMPEG_ORG_VER).tar.gz; \
	echo -e $(GREEN)"patching ffmpeg..."  $(DONE); \
	mv $(FFMPEG_ORG_VER) $(FFMPEG_VER); \
	patch -p0 < ffmpeg-3.4.1.patch; \
	tar -czf $(FFMPEG_TAR) $(FFMPEG_VER); \
	rm -rf $(FFMPEG_VER); \
	echo -e $(GREEN)"OK:generate patched kernel:$(FFMPEG_TAR)!"  $(DONE); \
	fi
	@if [ ! -f $(FFMPEG_TAR) -a -f $(FFMPEG_ORG_VER).tar.gz -a 'y' = $(FF_ENABLE_MINI) ]; then \
	tar xf $(FFMPEG_ORG_VER).tar.gz; \
	echo -e $(GREEN)"patching mini ffmpeg ..."  $(DONE); \
	mv $(FFMPEG_ORG_VER) $(FFMPEG_VER); \
	patch -p0 < ffmpeg-3.4.1_lowflash.patch; \
	tar -czf $(FFMPEG_TAR) $(FFMPEG_VER); \
	rm -rf $(FFMPEG_VER); \
	echo -e $(GREEN)"OK:generate patched kernel:$(FFMPEG_TAR)!"  $(DONE); \
	fi
	@if [ ! -f $(FFMPEG_TAR) -a ! -f $(FFMPEG_ORG_VER).tar.gz ]; then \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	echo -e $(RED) "| Cannot found the $(FFMPEG_ORG_VER) source file                                   |"$(DONE); \
	echo -e $(RED) "| Please download the compression package from http://ffmpeg.org/releases/    |"$(DONE); \
	echo -e $(RED) "| Copy $(FFMPEG_ORG_VER).tar.gz to thirdparty/ffmpeg                               |"$(DONE); \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	fi

$(ORG_FFMPEG): install_patch
	@if  [ ! -d $(FFMPEG_VER) -a -f $(FFMPEG_TAR) ]; then tar -xvf $(FFMPEG_TAR); fi
	@if  [ -d $(FFMPEG_VER) ]; then cd $@; chmod 777 ./* -R; ./$(FF_CONFIG_SH); cd -; fi
	@if  [ $(FF_ADAPT_LITEOS) = 'y' ]; then cd $@; ./adapt_liteos_config.sh; cd -; fi
	$(MAKE) $(MFLAGS) -j16 -C $(COMPILE_ROOT)/$@
	$(MAKE) $(MFLAGS) -C $(@) install
