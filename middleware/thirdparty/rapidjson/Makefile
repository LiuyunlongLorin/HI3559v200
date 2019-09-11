LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../..
include $(MIDDLEWARE_DIR)/base.mak

COMPILE_ROOT := $(LOCAL_DIR)

GREEN="\e[32;1m"
DONE="\e[39m"
RED="\e[31m"

SVER := v1.1.0
SOURCE_VER := rapidjson-1.1.0
SOURCE_DIR := rapidjson
SUBDIRS =$(SOURCE_DIR)
#if you want to exec configure of ffmpeg, open "cd $@;./build.sh"
#or we will disable it to  save compile time
SOURCE_URL := https://codeload.github.com/Tencent/rapidjson/tar.gz/v1.1.0

all: $(SUBDIRS)

.PHONY: $(SUBDIRS) clean

install: all

clean:
	rm -rf $(SOURCE_DIR)
	rm -rf $(SOURCE_VER)

source_download:
	@if [ ! -f $(SOURCE_VER).tar.gz ]; then curl -O --insecure $(SOURCE_URL); \
	mv v1.1.0 $(SOURCE_VER).tar.gz; \
	fi

install_patch: source_download
	@if [ ! -d $(SOURCE_VER) -a -f $(SOURCE_VER).tar.gz ]; then \
	tar xf $(SOURCE_VER).tar.gz; \
	echo -e $(GREEN)"patching source..."  $(DONE); \
	mv $(SOURCE_VER) $(SOURCE_DIR); \
	echo -e $(GREEN)"OK:generate patched $(SOURCE_VER)!"  $(DONE); \
	fi
	@if [ ! -d $(SOURCE_DIR) -a ! -f $(SOURCE_VER).tar.gz ]; then \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	echo -e $(RED) "| Cannot found the $(SOURCE_VER) source file                                   |"$(DONE); \
	echo -e $(RED) "| Please download the compression package from https://codeload.github.com/Tencent/rapidjson/tar.gz/v1.1.0    |"$(DONE); \
	echo -e $(RED) "| Copy $(SOURCE_VER).tar.gz to thirdparty/rapidjson                               |"$(DONE); \
	echo -e $(RED) "-------------------------------------------------------------------------------"$(DONE); \
	fi

$(SUBDIRS): install_patch
