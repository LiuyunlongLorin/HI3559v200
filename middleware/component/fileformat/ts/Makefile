LOCAL_DIR = $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..

include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

TS_LIB_PATH = $(shell pwd)/lib
TS_HEADER_PATH = $(shell pwd)/include
TS_LIB_FILE=$(TS_LIB_PATH)/libts.a
TS_DLL_FILE=$(TS_LIB_PATH)/libts.so
TS_HEADER_FILE=$(TS_HEADER_PATH)/*.h

all:    $(TS_LIB_FILE) $(TS_DLL_FILE) $(TS_HEADER_FILE)
install: all
	cp -rf $(TS_LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(TS_DLL_FILE) $(OUT_LIB_DIR)/
	cp -rf $(TS_HEADER_FILE) $(OUT_HEADER_DIR)/
# Clean this project
clean:
