LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

COMPILE_ROOT := $(LOCAL_DIR)

GREEN="\e[32;1m"
DONE="\e[39m"
RED="\e[31m"

SOURCE_CONFIG_SH := build.sh
SOURCE_VER := tiff-4.0.9
DEST_DIR := tiff
SUBDIRS =$(DEST_DIR)
SHARED_LIBS := $(LOCAL_DIR)/$(DEST_DIR)/_install/lib/libtiff.so
STATIC_LIBS := $(LOCAL_DIR)/$(DEST_DIR)/_install/lib/libtiff.a
#if you want to exec configure of ffmpeg, open "cd $@;./build.sh"
#or we will disable it to  save compile time
SOURCE_URL := http://download.osgeo.org/libtiff/$(SOURCE_VER).tar.gz

all: $(SUBDIRS)

.PHONY: $(SUBDIRS) clean

install: all
		cp -rf $(SHARED_LIBS) $(OUT_LIB_DIR)/
		cp -rf $(STATIC_LIBS) $(OUT_LIB_DIR)/
clean:
	rm -rf $(DEST_DIR)
	rm -rf $(SOURCE_VER)

source_download:
	@if [ ! -f $(SOURCE_VER).tar.gz ]; then curl -O $(SOURCE_URL); fi

install_patch: source_download
	@if [ ! -d $(DEST_DIR) -a -f $(SOURCE_VER).tar.gz ]; then \
	tar xf $(SOURCE_VER).tar.gz; \
	echo -e $(GREEN)"patching source..."  $(DONE); \
	mv $(SOURCE_VER) $(DEST_DIR); \
	patch -p0 < $(SOURCE_VER).patch; \
	rm -rf $(SOURCE_VER); \
	echo -e $(GREEN)"OK:generate patched $(DEST_DIR)!"  $(DONE); \
	fi
	@if [ ! -f $(SOURCE_VER).tar.gz ]; then \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	echo -e $(RED) "| Cannot found the $(SOURCE_VER) source file                                   |"$(DONE); \
	echo -e $(RED) "| Please download the compression package from http://download.osgeo.org/libtiff/    |"$(DONE); \
	echo -e $(RED) "| Copy $(SOURCE_VER).tar.gz to thirdparty/tiff                               |"$(DONE); \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	fi

$(SUBDIRS): install_patch
	@if  [ -d $(DEST_DIR) ]; then cd $@; chmod 777 ./* -R; ./$(SOURCE_CONFIG_SH) "$(SEC_CFLAGS)"; cd -; fi
	$(MAKE) $(MFLAGS) -j16 -C $(COMPILE_ROOT)/$@
	$(MAKE) $(MFLAGS) -C $(@) install