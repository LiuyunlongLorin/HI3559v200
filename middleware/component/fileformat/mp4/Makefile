LOCAL_DIR = $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..

include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

MP4_LIB_PATH = $(shell pwd)/lib
MP4_HEADER_PATH = $(shell pwd)/include
MP4_LIB_FILE=$(MP4_LIB_PATH)/libmp4.a
MP4_DLL_FILE=$(MP4_LIB_PATH)/libmp4.so
MP4_HEADER_FILE=$(MP4_HEADER_PATH)/*.h

all:    $(MP4_LIB_FILE) $(MP4_DLL_FILE) $(MP4_HEADER_FILE)
install: all
	cp -rf $(MP4_LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(MP4_DLL_FILE) $(OUT_LIB_DIR)/
	cp -rf $(MP4_HEADER_FILE) $(OUT_HEADER_DIR)/
# Clean this project
clean:
