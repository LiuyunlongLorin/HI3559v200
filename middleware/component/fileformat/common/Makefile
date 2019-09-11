LOCAL_DIR = $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..

include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

FILEFORMAT_LIB_PATH = $(shell pwd)/lib
FILEFORMAT_LIB_FILE=$(FILEFORMAT_LIB_PATH)/libfileformat.a
FILEFORMAT_DLL_FILE=$(FILEFORMAT_LIB_PATH)/libfileformat.so

all:    $(FILEFORMAT_LIB_FILE) $(FILEFORMAT_DLL_FILE)
install: all
	cp -rf $(FILEFORMAT_LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(FILEFORMAT_DLL_FILE) $(OUT_LIB_DIR)/
# Clean this project
clean:
