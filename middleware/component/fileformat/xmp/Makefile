LOCAL_DIR = $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..

include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

XMP_LIB_PATH = $(shell pwd)/lib
XMP_HEADER_PATH = $(shell pwd)/include
XMP_LIB_FILE=$(XMP_LIB_PATH)/libxmp.a
XMP_DLL_FILE=$(XMP_LIB_PATH)/libxmp.so
XMP_HEADER_FILE=$(XMP_HEADER_PATH)/*.h

all:    $(XMP_LIB_FILE) $(XMP_DLL_FILE) $(XMP_HEADER_FILE)
install: all
	cp -rf $(XMP_LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(XMP_DLL_FILE) $(OUT_LIB_DIR)/
	cp -rf $(XMP_HEADER_FILE) $(OUT_HEADER_DIR)/
# Clean this project
clean:
