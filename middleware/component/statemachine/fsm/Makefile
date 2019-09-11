LOCAL_DIR := $(shell pwd)
MIDDLEWARE_DIR := $(LOCAL_DIR)/../../..
include $(MIDDLEWARE_DIR)/base.mak

.PHONY: all clean install

LIB_PATH = $(shell pwd)/lib

LIB_FILE=$(LIB_PATH)/*.a
DLL_FILE=$(LIB_PATH)/*.so

all:    $(LIB_FILE) $(DLL_FILE)
install: all
	cp -rf $(LIB_FILE) $(OUT_LIB_DIR)/
	cp -rf $(DLL_FILE) $(OUT_LIB_DIR)/
# Clean this project
clean:
