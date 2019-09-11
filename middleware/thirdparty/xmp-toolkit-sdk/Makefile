LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

COMPILE_ROOT := $(LOCAL_DIR)

GREEN="\e[32;1m"
DONE="\e[39m"
RED="\e[31m"

SOURCE_VER := XMP-Toolkit-SDK-CC201607
SOURCE_DIR := xmp-toolkit-sdk
MODNAME := $(SOURCE_DIR)
SUBDIRS =$(MODNAME)
#if you want to exec configure of ffmpeg, open "cd $@;./build.sh"
#or we will disable it to  save compile time
SOURCE_URL := http://download.macromedia.com/pub/developer/xmp/sdk/$(SOURCE_VER).zip

DEP_SRC_VER :=expat-2.1.0
DEP_SRC_DEST_DIR := $(COMPILE_ROOT)/$(SOURCE_DIR)/third-party
DEP_SRC_URL := https://nchc.dl.sourceforge.net/project/expat/expat/2.1.0/$(DEP_SRC_VER).tar.gz


all: $(SUBDIRS)

.PHONY: $(SUBDIRS) clean

install: all
	@if  [ -d $(SOURCE_DIR) ]; then \
	pushd $(SOURCE_DIR); \
	make install; \
	popd; \
	fi

clean:
	rm -rf $(SOURCE_VER)
	rm -rf $(SOURCE_DIR)

source_download:
	@if [ ! -f $(SOURCE_VER).zip ]; then curl -O $(SOURCE_URL); fi
	@if [ ! -f $(DEP_SRC_VER).tar.gz ]; then curl -O $(DEP_SRC_URL); fi

install_patch: source_download
	@if [ ! -d $(SOURCE_DIR) -a -f $(SOURCE_VER).zip ]; then \
	unzip $(SOURCE_VER).zip; \
	echo -e $(GREEN)"patching source..." $(DONE); \
	mv $(SOURCE_VER) $(SOURCE_DIR); \
	fi
	@if [ -f $(DEP_SRC_VER).tar.gz ]; then \
	tar zxvf $(DEP_SRC_VER).tar.gz -C $(DEP_SRC_DEST_DIR); \
	pushd $(DEP_SRC_DEST_DIR); \
	mv $(DEP_SRC_VER) expat; \
	popd; \
	echo -e $(GREEN)"copy deps.."  $(DONE); \
	echo -e $(GREEN)"OK:patch $(DEP_SRC_VER) to $(SOURCE_DIR)!"  $(DONE); \
	echo -e $(GREEN)"OK:generate patched $(SOURCE_VER)!"  $(DONE); \
	fi
	patch -p0 < $(SOURCE_VER).patch;

	@if [ ! -f $(DEP_SRC_VER).tar.gz -o ! -f $(SOURCE_VER).zip ]; then \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	echo -e $(RED) "| Cannot found the $(SOURCE_VER) source file                                   |"$(DONE); \
	echo -e $(RED) "| Please download the compression package from $(DEP_SRC_URL)    |"$(DONE); \
	echo -e $(RED) "| Please download the compression package from &(SOURCE_URL)    |"$(DONE); \
	echo -e $(RED) "| Copy $(SOURCE_VER).tar.gz to thirdparty/xmp-toolkit-sdk                          |"$(DONE); \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	fi

$(SUBDIRS):
	@if [ ! -d $(SOURCE_DIR) ]; then \
	make install_patch; \
	fi
	@if  [ -d $(SOURCE_DIR) ]; then \
	cd $(SOURCE_DIR); \
	make; \
	cd -; \
	fi